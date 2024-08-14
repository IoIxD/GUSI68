import os,re
# assign directory
directory = '.'

files = {}
 
# iterate over files in
# that directory
for filename in os.listdir(directory):
    filename_full = os.path.join(directory, filename)
    # checking if it is a file
    if os.path.isfile(filename_full):
        f = open(filename_full,encoding="macroman")
        lines = "".join(f.readlines())
        
        definitions = {}

        matches = re.findall("<<(.*?)>>=((.|\n)*?)(@|<<(.*?)>>=)",lines) 
        
        for mat in matches:
            resolved = mat[1]
            if mat[0] in definitions:
                definitions[mat[0]] += resolved
            else:
                definitions[mat[0]] = resolved
  
        definitions_resolved = {}

        for defin in definitions:
            print(defin, "expanded size:", len(definitions[defin]),end="\r")
            matches = re.findall("<<(.*?)>>\n",definitions[defin])
            while len(matches) >= 1:
                print(defin, "expanded size: "+str(len(definitions[defin])),end="              \r")
                matches = re.findall("<<(.*?)>>\n",definitions[defin])
                for mat in matches:
                    if mat in definitions:
                        if len(definitions[defin]) >= 10000:
                            matches = []
                            break
                        print(defin, "expanded size: "+str(len(definitions[defin])),end="\r")
                        definitions[defin] = definitions[defin].replace("<<"+mat+">>", definitions[mat])
                    else:
                        definitions[defin] = definitions[defin].replace("<<"+mat+">>", "#warning unhandled macro \""+mat+"\"")
            print("")
            
        for defin in definitions:
            if ".h" in defin or ".cp" in defin:
                nfname = defin.replace(".cp",".cpp")
                fn = os.path.join(os.path.join(os.path.join("..",".."),"src"),nfname)
                os.makedirs(os.path.dirname(fn), exist_ok=True)
                nf = open(fn,"w+")
                nf.write(definitions[defin])
                print(nfname)

        f.close()