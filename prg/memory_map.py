#!/usr/bin/env python -u
# Author:
#
#-------------------------------------------------------------------------------
# Notes
#-------------------------------------------------------------------------------
# Python script to parse xml file and generate header file for memory maps

#-------------------------------------------------------------------------------
# Change Log
#-------------------------------------------------------------------------------


#inclusion of necessary modules
from xml.dom.minidom import parse
import argparse
import sys
import os
import string

def parseXmlGenerateDescriptions(inFile, outFile):
    optionsData = None
    
    outdescFileHdl = open(outFile, "w")
    #todo: add error check for file open
    if (outdescFileHdl <= 0) :
        print 'Invalid File handle'
        return
    outdescFileHdl.write('#if defined(MMAP_INCLUDE_PARTITION_INFO)  \n')

    dom4 = parse(inFile)
    elementList1 = dom4.getElementsByTagName('element')
    for element in elementList1:
        itemList = element.getElementsByTagName('partition')
        for item in itemList:
            itemList1 = item.getElementsByTagName('options')
            if(itemList1 != 'NULL'):
                for item1 in itemList1:
                    optionsData = 'MMAP_UPDATE_PARTITION_INFO_OPT(' + item.attributes['name'].value + ', ' + item1.attributes['opt'].value + ', ' + item1.attributes['value'].value + ')\n'
                    outdescFileHdl.write(optionsData)
            itemList2 = item.getElementsByTagName('attributes')
            if(itemList2 != 'NULL'):
                for item2 in itemList2:
                    attrData = 'MMAP_UPDATE_PARTITION_INFO_ATTRIBUTE(' + item.attributes['name'].value + ', ' + item2.attributes['attr'].value + ', ' + item2.attributes['value'].value + ')\n'
                    outdescFileHdl.write(attrData)

    outdescFileHdl.write('#endif \n')
    outdescFileHdl.close()
    return None

def parseflagdefines(flagdefines, name, outFileHdl):
    dom2 = parse(flagdefines)
    ret = -1
    elementList = dom2.getElementsByTagName('element')
    for element in elementList:
        itemList = element.getElementsByTagName('flags')
        # Loop till all the elements are parsed
        for item in itemList:
            if (item.attributes['name'].value == name):
                if(item.getAttribute('depends_on')):
                    define_value = item.attributes['depends_on'].value
                    define_value = define_value.split()
                    define = "#if "
                    for entry in define_value:
                        if entry == "and":
                            define = define + " && "
                        elif entry == "or":
                            define = define + " || "
                        elif entry == "not":
                            define = define + '!'
                        else:
                            define = define + 'defined(' + entry + ')'
                    define = define + "\n"
                    outFileHdl.write(define)
                    ret = 0
    return ret

def parse_BaseFile_defines(basefile, name):
    dom2 = parse(basefile)
    ret = 0
    elementList = dom2.getElementsByTagName('element')
    for element in elementList:
        itemList = element.getElementsByTagName('mmap')
        # Loop till all the elements are parsed
        for item in itemList:
            if (item.attributes['name'].value == name):
                return item.attributes['base'].value    #Already defined
    return None

def parseXmlGenerateDefines(inFile, outdefineFile, tagname):
    outFileHdlnew = open(outdefineFile, "w")
    #todo: add error check for file open
    if (outFileHdlnew <= 0) :
        print 'Invalid File handle'
        return
    dom3 = parse(inFile)
    top_element = dom3.documentElement
    if(top_element.getAttribute('base')):
        base = top_element.attributes['base'].value
        base_val = int(base, 16)
    else:
        base = "0x00000000"
    if(top_element.getAttribute('size')):
        size_ptr = top_element.attributes['size'].value
        if (string.find(size_ptr, "KB") != -1) :
            size = string.rstrip(size_ptr, "KB")
            size_val = (int(size) * 1024)
            size = str(hex(size_val))
        elif (string.find(size_ptr, "MB") != -1) :
            size = string.rstrip(size_ptr, "MB")
            size_val = (int(size) * 1048576)
            size = str(hex(size_val))
        else:
            size = size_ptr
            size_val = int(size, 16)
    else:
        size = "0x40000000"
               
    if (base != None) and (size != "NULL"):
        end_val = base_val + size_val
        end = str(hex(end_val))
    else:
        end = None

    base_string = '#define RAM_CONFIG_PSTART'
    size_string = '#define RAM_CONFIG_SIZE'
    end_string  = '#define RAM_CONFIG_PEND'

    curr_base = base_string+' '+base+'\n'
    curr_size = size_string+' '+size+'\n'
    curr_end = end_string+' '+end+'\n'

    outFileHdlnew.write(curr_base)
    outFileHdlnew.write(curr_size)
    outFileHdlnew.write(curr_end)

    elementList = dom3.getElementsByTagName('element')
    for element in elementList:
        # parsing the agruments for each element - memory_map_name, base, size and flag dependents
        
        itemList = element.getElementsByTagName(tagname)
        # Loop till all the elements are parsed
        for item in itemList:
            if(item.getAttribute('base')):
                print 'Defines parsing'
                print(item.getAttribute('base'))
                base = item.attributes['base'].value
                base_val =int(base, 16)
            else:
                base = None
        
            if(item.getAttribute('size')):
                size_ptr = item.attributes['size'].value
                if (string.find(size_ptr, "KB") != -1) :
                    size = string.rstrip(size_ptr, "KB")
                    size_val = (int(size) * 1024)
                    size = str(hex(size_val))
                elif (string.find(size_ptr, "MB") != -1) :
                    size = string.rstrip(size_ptr, "MB")
                    size_val = (int(size) * 1048576)
                    size = str(hex(size_val))
                else:
                    size = size_ptr
                    size_val = int(size, 16)
            else:
                size = "NULL"
                
            if (base != None) and (size != "NULL"):
                end_val = base_val + size_val
                end = str(hex(end_val))
            else:
                end = None

            base_string = '#define RAM_CONFIG'+'_'+item.attributes['name'].value+'_PSTART'
            size_string = '#define RAM_CONFIG'+'_'+item.attributes['name'].value+'_SIZE'
            end_string  = '#define RAM_CONFIG'+'_'+item.attributes['name'].value+'_PEND'

            curr_base = base_string+' '+base+'\n'
            curr_size = size_string+' '+size+'\n'
            curr_end = end_string+' '+end+'\n'

            if (base != None):
                outFileHdlnew.write(curr_base)
            if (size != "NULL"):
                outFileHdlnew.write(curr_size)
            if (end != None):
                outFileHdlnew.write(curr_end)

    print 'System defines creation done !!!'


def parseXmlGenerateMemoryMap(inFile, outFile, tagname, basefile, flagfile):
    dynamicpartitionid = 'NULL'
    backupdestinationnid = 'NULL'
    backupreqid = 'NULL'
    outFileHdl = open(outFile, "a")
    #todo: add error check for file open
    if (outFileHdl <= 0) :
        print 'Invalid File handle'
        return

    dom1 = parse(inFile)
    elementList = dom1.getElementsByTagName('element')
    for element in elementList:
        elementName = element.attributes['name'].value.upper()
        outFileHdl.write('\n/*'+'-'*76+'*/\n')
        outFileHdl.write('/*'+elementName+'*/\n')
        outFileHdl.write('/*'+'-'*76+'*/\n')

        # parsing the agruments for each element - memory_map_name, base, size and flag dependents
        
        itemList = element.getElementsByTagName(tagname)
        # Loop till all the elements are parsed
        for item in itemList:
            if (item.getAttribute('dynamic_partition_id')):
                if (item.attributes['dynamic_partition_id'].value == "TRUE") and (dynamicpartitionid =='NULL'):
                    dynamicpartitionid = '#define MEM_DYNAMIC_PARTITION_ID  ' + 'PARTITION_ID_' + item.attributes['name'].value + '\n'
                    outFileHdl.write(dynamicpartitionid)
                else:
                    if (item.attributes['dynamic_partition_id'].value == "TRUE") and (dynamicpartitionid !='NULL'):
                        print 'more than one dynamic partition is set'
                        return

            if (item.getAttribute('backup_destination_id')):
                if ((item.getAttribute('backup_destination_id') == "TRUE") and (backupdestinationnid =='NULL')):
                    backupdestinationnid = '#define BACKUP_DEST_PARTITION_ID  ' + 'PARTITION_ID_' + item.attributes['name'].value + '\n'
                    outFileHdl.write(backupdestinationnid)
                else:
                    if (item.attributes['backup_destination_id'].value == "TRUE") and (backupdestinationnid !='NULL'):
                        print 'more than one backup destination is set'
                        return

            if (item.getAttribute('backup_required_id')):
                if (item.getAttribute('backup_required_id') == "TRUE"):
                    backupreqid = '#if defined(MMAP_INCLUDE_PARTITION_INFO)  \n' + 'MMAP_UPDATE_PARTITION_INFO_OPT(' + item.attributes['name'].value + ', BACKUP_DATA_REQUIRED, TRUE)' + '\n' + '#endif \n'
                    #backupreqid = '#define BACKUP_DATA_REQ_PARTITION_ID  ' + 'PARTITION_ID_' + item.attributes['name'].value + '\n'
                    outFileHdl.write(backupreqid)

            if (item.getAttribute('nvm_partition_disable')):
                if (item.attributes['nvm_partition_disable'].value == "yes"):
                    nvm_partition_ctrl = '#define NVM_PARTITION_DISABLE  1\n'
                    outFileHdl.write(nvm_partition_ctrl)
           
            if None == flagfile:
                flagret = -1
            else:
                flagret = parseflagdefines(flagfile,item.attributes['name'].value, outFileHdl)

            
            #Parsing the base value - inputs can be +0, +, <abs_addr>, no
            # +0 -         Adds 0 to previous section base
            # +  -         Considers the element base as section base
            # <abs_addr> - considered as base
            #if base not mentioned considers ID as the base 
            if(item.getAttribute('base')):
                print(item.getAttribute('base'))
                base = item.attributes['base'].value
            else: #check for region map or direct definition in base file else assign Partition ID as base
                base = 'PARTITION_ID_' + item.attributes['name'].value
                if (basefile != None):
                    if (item.getAttribute('region')):
                        print item.attributes['region'].value
                        ret = parse_BaseFile_defines(basefile, item.attributes['region'].value)
                        print ret
                    else:
                        ret = parse_BaseFile_defines(basefile, item.attributes['name'].value)
                    if (ret != None):
                        base = ret

            # Parsing the sizes - inputs can be <abs_val>, <val KB>, <val MB>
            # abs_val - Puts the absolute value
            # KB, MB are decoded to 1024, 1048576
            if(item.getAttribute('size')):
                size_ptr = item.attributes['size'].value
                if (string.find(size_ptr, "KB") != -1) :
                    size = string.rstrip(size_ptr, "KB")
                    size_val = hex(int(size) * 1024)
                    size = str(size_val)
                elif (string.find(size_ptr, "MB") != -1) :
                    size = string.rstrip(size_ptr, "MB")
                    size_val = hex(int(size) * 1048576)
                    size = str(size_val)
                else:
                    size = size_ptr
            else:
                size = "NULL"
            
            base_string = 'MMAP'+'_'+item.attributes['name'].value+'_BASE'
            size_string = 'MMAP'+'_'+item.attributes['name'].value+'_SIZE'

            curr_base = '#define  '+base_string+'    '+base+'\n'
            curr_size = '#define  '+size_string+'    '+size+'\n'

            outFileHdl.write(curr_base)
            if (size != "NULL"):
                outFileHdl.write(curr_size)
          
            if(flagret != -1):
                endif = '#endif\n'
                outFileHdl.write(endif)

    outFileHdl.flush()
    outFileHdl.close()

def file_extension_verification(filename):
    fileName, fileExtension = os.path.splitext(filename)
    if '.xml' != fileExtension:
        print 'wrong extension' + fileName + fileExtension
        return

def entry_function():
    parser = argparse.ArgumentParser(description='Script to generate memory map header from xml')
    #parser.add_argument('-i', '--ifile',   nargs = 2  help="Input xml file ",   required=False, default='base.xml')
    parser.add_argument('-i', '--ifile',  help="Input xml file ",   required=False, default='Partition.xml')
    parser.add_argument('-b', '--bfile',  help="(OPTIONAL) Base XML file",   required=False)
    parser.add_argument('-d', '--dfile',  help="(OPTIONAL) Flag defines file",   required=False, default='flag_dependencies.xml')
    parser.add_argument('-o', '--ofile',    help='(OPTIONAL) Output file name',  required=False, default='memory_map.h')
    parser.add_argument('-c', '--cfile',    help='(OPTIONAL) Output file name',  required=False, default='memory_map_description.h')
    parser.add_argument('-D', '--Dfile',    help='(OPTIONAL) Output Define file name',  required=False, default='ram_layout.h')
    args = parser.parse_args()
    print args
    print args.ifile
    print args.bfile
    print args.dfile
    print args.Dfile
    print args.cfile
    print 'XML parsing'
    
    if None == args.ifile:
        print 'Please specify input file'
        return
    file_extension_verification(args.ifile)

    if None != args.bfile:
        file_extension_verification(args.bfile)
    if None != args.dfile:
        file_extension_verification(args.dfile)
    
    outFile = args.ofile
    outFileHdl = open(outFile, "w")
    #todo: add error check for file open
    if (outFileHdl <= 0) :
        print 'Invalid File handle'
        return
    outFileHdl.close()

    parseXmlGenerateMemoryMap(args.ifile, outFile, 'partition', args.bfile, args.dfile)
    parseXmlGenerateDescriptions(args.ifile, args.cfile)
    if ((None != args.Dfile) and (None != args.cfile)):
        parseXmlGenerateDefines(args.bfile, args.Dfile, 'mmap')

    return

##------------------------------Main Function- ---------------------------------
if __name__ == "__main__":

    entry_function()
    sys.exit()

