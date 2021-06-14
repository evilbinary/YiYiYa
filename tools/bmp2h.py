# coding:utf-8
import os
import sys
from PIL import Image

def main(file,color_format):
    filename=os.path.splitext(file)[0]
    e=os.path.splitext(file)[1]
    if e==".bmp":
        img=Image.open(file)
        
        x,y=img.size
        
        s="/*\n*\n* evilbinary.\n*\n*/\n\n"
        s+="#ifndef "+filename.upper()+"_H\n#define "+filename.upper()+"_H\n\n"
  
        
        if color_format=='RGB111':
            _bytes=0
            _pixels=2
        elif color_format=='RGB332':
            _bytes=1
            _pixels=1
        elif color_format=='RGB565':
            _bytes=2
            _pixels=1
        elif color_format=='RGB888':                   
            _bytes=3
            _pixels=1
        else:
            _bytes=4
            _pixels=0
            
        s+="const unsigned char "+filename+"_bmp[]={\n"


        for j in range(y):
            for i in range(x):
                r,g,b=img.getpixel((i,j))
                if color_format=="RGB111":
                    if((i)%2==0):
                        r=r>>7
                        g=g>>7
                        b=b>>7
                        s+="0x%0.1X"%(((r<<2)+(g<<1)+b)) #_B1G1R1_B1G1R1
                        if(i<x):
                            r,g,b=img.getpixel((i+1,j))
                            r=r>>7
                            g=g>>7
                            b=b>>7
                            s+="%0.1X"%(((r<<2)+(g<<1)+b)) 
                    else:
                       continue
     
                elif color_format=="RGB332":
                    r=r>>5
                    g=g>>5
                    b=b>>6
                    s+="0x%0.2X"%(b+(g<<2)+(r<<5))  #R3G3B2
                elif color_format=="RGB565":
                    r=r>>3
                    g=g>>2
                    b=b>>3
                    s+="0x%0.2X"%(((g&0x07)<<5)+b)+","      #G3B5
                    s+="0x%0.2X"%(((g&0x38)>>3)+(r<<3))     #R5G3
                else:                   
                    s+="0x%0.2X"%(b)+","            
                    s+="0x%0.2X"%(g)+","
                    s+="0x%0.2X"%(r)
                if j==y-1 and i==x-(_pixels): 
                    s+="\n};"
                else:
                    s+=","
            s+="\n" 
        
        # s+="\nconst image_t "+filename+"={"
        # s+=str(x)+","+str(y)+","+str(_bytes)+","+str(_pixels)+","+filename+"_bmp};\n" 
        s+="\n#endif\n"        

        f= open(filename+".h","w+")
        f.write(s)
        f.close() 
    else:
        print("Not a bitmap!")
         
        
if __name__== "__main__":
    file = sys.argv[1]
    color_format = sys.argv[2]
    main(file, color_format)