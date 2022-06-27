import os


files = ['Cameraman.bmp', 'Lena.bmp', 'Goldhill.bmp']
params = [[1,0,0],[0,1,0],[0,0,1],[1,1,-1],[1,.5,-.5],[.5,1,-.5],[.5,.5,0]]
qlen = range(8,0,-1)
cmd = 'Exp2 -f {0} -l {1} -a {2} -b {3} -c {4}'
#for q in qlen:
    #paras = params[0]
    #os.system(cmd.format(files[1], q, paras[0], paras[1], paras[2]))
#for file in files:
    #for q in qlen:
        #paras = params[0]
        #os.system('Exp2 -f {0} -l {1} -a {2} -b {3} -c {4}'.format(file, q, paras[0], paras[1], paras[2]))
for paras in params:
    os.system(cmd.format(files[2], 3, paras[0], paras[1], paras[2]))
