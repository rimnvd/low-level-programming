import binascii
import winreg
from winreg import *

data_type_names = {
    winreg.REG_SZ: "REG_SZ",
    winreg.REG_EXPAND_SZ: "REG_EXPAND_SZ",
    winreg.REG_MULTI_SZ: "REG_MULTI_SZ",
    winreg.REG_DWORD: "REG_DWORD",
    winreg.REG_QWORD: "REG_QWORD",
    winreg.REG_BINARY: "REG_BINARY",
    winreg.REG_DWORD_LITTLE_ENDIAN: "REG_DWORD_LITTLE_ENDIAN",
    winreg.REG_DWORD_BIG_ENDIAN: "REG_DWORD_BIG_ENDIAN",
    winreg.REG_LINK: "REG_LINK",
    winreg.REG_NONE: "REG_NONE",
    winreg.REG_QWORD_LITTLE_ENDIAN: "REG_QWORD_LITTLE_ENDIAN",
    winreg.REG_RESOURCE_LIST: "REG_RESOURCE_LIST",
    winreg.REG_FULL_RESOURCE_DESCRIPTOR: "REG_FULL_RESOURCE_DESCRIPTOR",
    winreg.REG_RESOURCE_REQUIREMENTS_LIST: "REG_RESOURCE_REQUIREMENTS_LIST",
}

aReg = ConnectRegistry(None, HKEY_LOCAL_MACHINE)

output = open("out.txt", 'a', encoding='utf-8')

i = 0

def my_write(string):
    global i
    string = string.split()

    output.write(
        f'{i} parent={string[0]} name={string[1]} type={string[2]} value_type={string[4]} value={string[3]}\n')
    #output.write(f'db.insert({{parent: 1}}, {{parent_name: "{string[0]}", name: "{string[1]}", type: "{string[2]}", '
   #              f'value_type: "{string[4]}", value: "{string[3]}"}})\n')
    i += 1


def func(path):
    if path == '':
        my_write("None HKEY_LOCAL_MACHINE key None REG_SZ")
    elif path.find('\\') == -1:
        my_write("HKEY_LOCAL_MACHINE " + path.split('\\')[-1] + " key None REG_SZ")
    else:
        my_write(path.split('\\')[-2] + " " + path.split('\\')[-1] + " key None REG_SZ")
    aKey = OpenKey(aReg, path)
    count = QueryInfoKey(aKey)[1]
    for i in range(count):
        val_name, val, val_type = EnumValue(aKey, i)
        if val_type == 3 and val is not None:
            val = binascii.hexlify(val).decode('utf-8')
        val_type = data_type_names.get(val_type)
        parent = path.split('\\')[-1]
        my_write(f'{parent} {val_name} value {val} {val_type}')
    for j in range(1024):
        old_path = path
        try:
            keyname = EnumKey(aKey, j)
            if len(path) == 0:
                path = keyname
            else:
                path = rf'{path}\{keyname}'
            func(path)
        except WindowsError:
            pass
        finally:
            path = old_path
func(r"")
output.close()
