//#define PCI_ADDR 0x62000000
#define PCI_ADDR 0x70000000
unsigned int prom_read_long(unsigned int address)
{
	uint32_t data=ps_read_32(address);
	if(address>=PCI_ADDR)
		printf("prom_read_long: 0x%08x 0x%08x\n",address,data);
	return(data);
}
unsigned int prom_read_word(unsigned int address)
{
	uint32_t data=ps_read_16(address);
	if(address>=PCI_ADDR)
		printf("prom_read_word: 0x%08x 0x%08x\n",address,data);
	return(data);
}
unsigned int prom_read_byte(unsigned int address)
{
	uint32_t data=ps_read_8(address);
	if(address>=PCI_ADDR)
		printf("prom_read_byte: 0x%08x 0x%08x\n",address,data);
	return(data);
}
void prom_m68k_write_memory_8(unsigned int address, unsigned int value)
{
	if(address>=PCI_ADDR)
		printf("prom_write_byte: 0x%08x 0x%08x\n",address,value);
	ps_write_8(address,value);
}
//void m68k_write_memory_16(uint32_t address, uint32_t value)
void prom_m68k_write_memory_16(unsigned int address, unsigned int value)
{
	if(address>=PCI_ADDR)
		printf("prom_write_word: 0x%08x 0x%08x\n",address,value);
	ps_write_16(address,value);
}
//void m68k_write_memory_32(uint32_t address, uint32_t value)
void prom_m68k_write_memory_32(unsigned int address, unsigned int value)
{
	if(address>=PCI_ADDR)
		printf("prom_write_long: 0x%08x 0x%08x\n",address,value);
	ps_write_32(address,value);
}
