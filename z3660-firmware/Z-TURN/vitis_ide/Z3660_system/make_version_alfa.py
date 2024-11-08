#!/usr/bin/env python

__author__   = 'shanshe'
__version___ = '1.0.1'
__date___    = '06.05.24'

import sys
import os
import shutil
import struct

DST_PATH = "Z:/z3660/alfa/"
SRC_PATH = "Alfa/sd_card/"
BOOT_NAME = "BOOT.BIN"
SCSI_NAME = "z3660_scsi.rom"
SOURCE = "C:/Users/shanshe/workspace/Z3660/src/version.h"
SOURCE_ALFA = "C:/Users/shanshe/workspace/Z3660/src/alfa.txt"
JED_NAME  =  "z3660.jed"

def buscarDefine(clave, archivo):
    claveCompleta = "#define " + clave
    for linea in archivo:
        if claveCompleta in linea:
            if linea.startswith("//") == False:
                listaTokens = linea.split()  # Dividimos cada linea en tokens
                return (listaTokens[2])


def limpiarValor(valor):
    valor = str(valor).replace("+", "")
    valor = str(valor).replace("L", "")
    valor = str(valor).replace("0x", "")
    valor = str(valor).replace('"', "")
    return (valor)


def getValor(clave,archivo):
    return (limpiarValor(buscarDefine(clave, archivo)))

def delete(src):
    if os.path.exists(src):
        os.remove(src)

def swap32(i):
    return struct.unpack("<i", struct.pack(">i", i))[0]

def crc32(name):
    with open(DST_PATH + name, "rb") as f:
        while True:
            data = f.read(4)
            if len(data) < 4:
                return
            data_int = struct.unpack("<i", data)[0]
            yield swap32(data_int)

def main():
    
    shutil.copy2(SRC_PATH + BOOT_NAME,DST_PATH)
    shutil.copy2(SRC_PATH + "../../z3660_scsi.rom",DST_PATH)
    shutil.copy2(SRC_PATH + "../../z3660.jed",DST_PATH)

    file = open(SOURCE, 'r', encoding='utf-8')
    V_MAJOR        = getValor("REVISION_MAJOR", file)
    V_MINOR        = getValor("REVISION_MINOR", file)
    BETA           = getValor("REVISION_BETA", file)
    file.close()

    #increment alfa number version
    if os.path.exists(SOURCE_ALFA):
        file = open(SOURCE_ALFA, 'r', encoding='utf-8')
        ALFA           = getValor("REVISION_ALFA", file)
        file.close()
    else:
        ALFA = "0"
    ALFA_INT = int(ALFA) + 1
    file = open(SOURCE_ALFA, 'w', encoding='utf-8')
    file.write("#define REVISION_ALFA " + str(ALFA_INT))
    file.close()
    
    # Clean
    if not os.path.exists(DST_PATH):
        print("Making DST_PATH ...")
        os.mkdir(DST_PATH)
    
    #########################
    # BOOT.BIN
    #########################
    delete(DST_PATH + "version.txt")
    text_file = open(DST_PATH + "version.txt", "w")
    version = "" + V_MAJOR + "."
    if (int(V_MINOR) < 10):
        version = version + "0"
    version = version + V_MINOR
    if (int(BETA) != 0):
        version = version + " BETA " + BETA + " ALFA " + ALFA
    print("version " + version)
    
    text_file.write("version=" + version + "\r")
    str_len = "%d" % os.path.getsize(DST_PATH + BOOT_NAME)
    text_file.write("length=" + str_len + "\r")
    checksum32=0
    for eachdata in crc32(BOOT_NAME):
        checksum32 = eachdata + checksum32
    str_checksum32 = "%08lX" % (checksum32 & 0xFFFFFFFF)
    text_file.write("checksum32=" + str_checksum32 + "\r")
    text_file.close()

    #########################
    # Z3660_SCSI.ROM
    #########################
    delete(DST_PATH + "scsirom_version.txt")
    text_file = open(DST_PATH + "scsirom_version.txt", "w")
    
    with open(DST_PATH + SCSI_NAME, 'rb') as f:
        s = f.read()
        pos=s.find(b"$VER")
        f.seek(pos+35)
        bytes = f.read(4)
        version_scsirom = "".join(map(chr,bytes))

    text_file.write("version=" + version_scsirom + "\r")
    str_len = "%d" % os.path.getsize(DST_PATH + SCSI_NAME)
    text_file.write("length=" + str_len + "\r")
    checksum32=0
    for eachdata in crc32(SCSI_NAME):
        checksum32 = eachdata + checksum32
    str_checksum32 = "%08lX" % (checksum32 & 0xFFFFFFFF)
    text_file.write("checksum32=" + str_checksum32 + "\r")
    text_file.close()
    
    print("z3660_scsi.rom checksum-32 " + str_checksum32)

    #########################
    # Z3660.JED
    #########################
    delete(DST_PATH + "jed_version.txt")
    text_file = open(DST_PATH + "jed_version.txt", "w")
    
    with open(DST_PATH + JED_NAME, 'rb') as f:
        s = f.read()
        pos=s.find(b"Date")
        f.seek(pos+16)
        bytes = f.read(24)
        version_jed = "".join(map(chr,bytes))

    text_file.write("version=" + version_jed + "\r")
    str_len = "%d" % os.path.getsize(DST_PATH + JED_NAME)
    text_file.write("length=" + str_len + "\r")
    checksum32=0
    for eachdata in crc32(JED_NAME):
        checksum32 = eachdata + checksum32
    str_checksum32 = "%08lX" % (checksum32 & 0xFFFFFFFF)
    text_file.write("checksum32=" + str_checksum32 + "\r")
    text_file.close()
    print("z3660.jed Date: " + version_jed)
    print("z3660.jed checksum-32 " + str_checksum32)

main()

