#filename="build.conf"

#import sys
#print ('argument list', sys.argv)
#if len(sys.argv)>1:
#    filename = sys.argv[1]


def file2hex(filename):
    filesize=0
    arraydata=[]

    with open(filename, 'rb') as f:
        data = f.read(16)
        while data:
            filesize+=len(data)
            s="";
            for d in data:
                s+=" 0x"+f"{d:02x},".upper()
            arraydata.append("   "+s)
            data = f.read(16)
    return {"filesize":filesize,"hexdata":arraydata}

print("#ifndef __EN8811H_MD32_H")
print("#define __EN8811H_MD32_H")

dm=file2hex("EthMD32.dm.bin")
print(f"const long int EthMD32_dm_size = {dm.get('filesize')};") #DM
print(f"const unsigned char EthMD32_dm[dm.get('filesize')] = {{")
print("\n".join(dm.get("hexdata")))
print("};")

dsp=file2hex("EthMD32.DSP.bin")
print(f"const long int EthMD32_pm_size = {dsp.get('filesize')};") #DSP
print(f"const unsigned char EthMD32_pm[dsp.get('filesize')] = {{")
print("\n".join(dsp.get("hexdata")))
print("};")

print("#endif /* End of __EN8801H_MD32_H */")
