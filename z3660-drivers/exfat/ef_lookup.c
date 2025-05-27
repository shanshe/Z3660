/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright © 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright © 2006 Marek Szyprowski
  Copyright © 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright © 2010-2013 Andrew Nayenko


  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

--------------------------------------------------------------------------*/

#include "include.h"

/*------------------------------------------------------------------------*/

int exfat_opendir(struct exFAT* ef, struct exFAT_Node* dir, struct exFAT_Iterator* ei)
{
	int rc;

	exfat_get_node(dir);
	ei->ei_Parent = dir;
	ei->ei_Current = NULL;
	rc = exfat_cache_directory(ef, dir);
	if (rc != 0)
		exfat_put_node(ef, dir);
	return rc;
}

void exfat_closedir(struct exFAT* ef, struct exFAT_Iterator* ei)
{
	exfat_put_node(ef, ei->ei_Parent);
	ei->ei_Parent = NULL;
	ei->ei_Current = NULL;
}

struct exFAT_Node* exfat_readdir(struct exFAT* ef, struct exFAT_Iterator* ei)
{
	if (ei->ei_Current == NULL)
		ei->ei_Current = ei->ei_Parent->en_Child;
	else
		ei->ei_Current = ei->ei_Current->en_Next;

	if (ei->ei_Current != NULL)
		return exfat_get_node(ei->ei_Current);
	else
		return NULL;
}

/*------------------------------------------------------------------------*/

static int lookup_cluster(struct exFAT* ef, struct exFAT_Node *parent, struct exFAT_Node** node, ULONG cluster, ULONG index)
{
	struct exFAT_Iterator ei;
	int rc;

	rc = exfat_opendir(ef, parent, &ei);
	if (rc != 0)
		return rc;
	while ((*node = exfat_readdir(ef, &ei)))
	{
		if ((*node)->en_StartCluster == cluster && (*node)->en_FPtrIndex == index)
		{
			exfat_closedir(ef, &ei);
			return 0;
		}
		if ((*node)->en_Flags & EXFAT_ATTRIB_DIR) {
			if ((rc = lookup_cluster(ef, *node, node, cluster, index)) != 0) {
				exfat_closedir(ef, &ei);
				return rc;
			}
		}
		exfat_put_node(ef, *node);
	}
	exfat_closedir(ef, &ei);
	return -ERROR_OBJECT_NOT_FOUND;
}

int exfat_lookup_cluster(struct exFAT* ef, struct exFAT_Node** node, ULONG cluster, ULONG index)
{
	struct exFAT_Node *parent;
	int rc;

	parent = exfat_get_node(ef->ef_Root);

	rc = lookup_cluster(ef, parent, node, cluster, index);

	exfat_put_node(ef, parent);
	return rc;
}

/*------------------------------------------------------------------------*/

static int compare_char(struct exFAT* ef, UWORD a, UWORD b)
{
	if (a >= ef->ef_UpCaseChars || b >= ef->ef_UpCaseChars)
		return (int) a - (int) b;

	return (int) le16_to_cpu(ef->ef_UpCase[a]) - (int) le16_to_cpu(ef->ef_UpCase[b]);
}

static int compare_name(struct exFAT* ef, const le16_t* a, const le16_t* b)
{
	while (le16_to_cpu(*a) && le16_to_cpu(*b))
	{
		int rc = compare_char(ef, le16_to_cpu(*a), le16_to_cpu(*b));
		if (rc != 0)
			return rc;
		a++;
		b++;
	}
	return compare_char(ef, le16_to_cpu(*a), le16_to_cpu(*b));
}

static int lookup_name(struct exFAT* ef, struct exFAT_Node* parent, struct exFAT_Node** node, const char* name, size_t n)
{
	struct exFAT_Iterator ei;
	le16_t buffer[EXFAT_NAME_MAX + 1];
	int rc;

	*node = NULL;

	rc = utf8_to_utf16(buffer, name, EXFAT_NAME_MAX, n);
	if (rc != 0)
		return rc;

	rc = exfat_opendir(ef, parent, &ei);
	if (rc != 0)
		return rc;
	while ((*node = exfat_readdir(ef, &ei)))
	{
		if (compare_name(ef, buffer, (*node)->en_Name) == 0)
		{
			exfat_closedir(ef, &ei);
			return 0;
		}
		exfat_put_node(ef, *node);
	}
	exfat_closedir(ef, &ei);
	return -ERROR_OBJECT_NOT_FOUND;
}

static size_t get_comp(const char* path, const char** comp)
{
	const char* end;

	*comp = path + strspn(path, "/"); /* skip leading slashes */
	end = strchr(*comp, '/');
	if (end == NULL)
		return strlen(*comp);
	else
		return end - *comp;
}

int exfat_lookup(struct exFAT* ef, struct exFAT_Node** node, const char* path)
{
	struct exFAT_Node* parent;
	const char* p;
	size_t n;
	int rc;

	/* start from the root directory */
	parent = *node = exfat_get_node(ef->ef_Root);
	for (p = path; (n = get_comp(p, &p)); p += n)
	{
		if (n == 1 && *p == '.') /* skip "." component */
			continue;
		rc = lookup_name(ef, parent, node, p, n);
		if (rc != 0)
		{
			exfat_put_node(ef, parent);
			return rc;
		}
		exfat_put_node(ef, parent);
		parent = *node;
	}
	return 0;
}

/*------------------------------------------------------------------------*/

static BOOL is_last_comp(const char* comp, size_t length)
{
	const char* p = comp + length;

	return get_comp(p, &p) == 0;
}

static BOOL is_allowed(const char* comp, size_t length)
{
	size_t i;

	for (i = 0; i < length; i++)
		switch (comp[i])
		{
		case 0x01 ... 0x1f:
		case '/':
		case '\\':
		case ':':
		case '*':
		case '?':
		case '"':
		case '<':
		case '>':
		case '|':
			return FALSE;
		}
	return TRUE;
}

int exfat_split(struct exFAT* ef, struct exFAT_Node** parent, struct exFAT_Node** node, le16_t* name, const char* path)
{
	const char* p;
	size_t n;
	int rc;

	memset(name, 0, (EXFAT_NAME_MAX + 1) * sizeof(le16_t));
	*parent = *node = exfat_get_node(ef->ef_Root);
	for (p = path; (n = get_comp(p, &p)); p += n)
	{
		if (n == 1 && *p == '.')
			continue;
		if (is_last_comp(p, n))
		{
			if (!is_allowed(p, n))
			{
				/* contains characters that are not allowed */
				exfat_put_node(ef, *parent);
				return -ERROR_OBJECT_NOT_FOUND; //ENOENT;
			}
			rc = utf8_to_utf16(name, p, EXFAT_NAME_MAX, n);
			if (rc != 0)
			{
				exfat_put_node(ef, *parent);
				return rc;
			}

			rc = lookup_name(ef, *parent, node, p, n);
			if (rc != 0 && rc != -ERROR_OBJECT_NOT_FOUND) //-ENOENT
			{
				exfat_put_node(ef, *parent);
				return rc;
			}
			return 0;
		}
		rc = lookup_name(ef, *parent, node, p, n);
		if (rc != 0)
		{
			exfat_put_node(ef, *parent);
			return rc;
		}
		exfat_put_node(ef, *parent);
		*parent = *node;
	}
	//exfat_fatal("impossible");
	return -ERROR_OBJECT_NOT_FOUND; 
}

