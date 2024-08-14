import os,re
# assign directory
directory = './src'

files = {}
 
# iterate over files in
# that directory
for filename in os.listdir(directory):
    if ".cpp" not in filename:
        continue
    filename_full = os.path.join(directory, filename)
    # checking if it is a file
    if os.path.isfile(filename_full):
        f = open(filename_full,encoding="macroman")
        lines = "".join(f.readlines())

        matches = re.findall("(.*?)(\((.*?)\))\n{",lines)

        defs = ""
        for mat in matches:
            defs += mat[0]+mat[1]+";"
            pass

        of = open(filename_full.replace(".cpp",".h"),"r+")

        olines = "".join(of.readlines())
        matches = re.findall("#ifndef (.*?)\n#define (.*?)\n",olines)
        
        for mat in matches:
            olines = olines.replace("#define "+mat[1], "#define "+mat[1]+"\n"+defs)
            print(mat[0]+mat[1])

        olines = olines.replace("\x00","")
        
        of.truncate(0)
        of.seek(0)
        of.write(olines)
        f.close()