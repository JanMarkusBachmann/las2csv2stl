import math
import tkinter.filedialog

class Vektor:
    def __init__(self, x1, x2, y1, y2, z1=0.0, z2=0.0):
        self.x1 = x1
        self.x2 = x2
        self.y1 = y1
        self.y2 = y2
        self.z1 = z1
        self.z2 = z2
        self.z = self.z2 - self.z1
        self.x = self.x2 - self.x1
        self.y = self.y2 - self.y1
        self.pikk = math.sqrt(((self.x) ** 2) + ((self.y) ** 2))
        self.pikkz = math.sqrt(((self.x)**2)+((self.y)**2)+(self.z)**2)
        if self.pikk != 0:
            self.kaal = 1/((self.pikk*10)**2)
        else:
            self.kaal = 0

    def __str__(self):
        return f'Vektor ({self.x}, {self.y}, {self.z}) pikusega {round(self.pikk), 2}'

class Punkt:
    def __init__(self, x, y, z=0.0, is_edge=False):
        self.x = x
        self.y = y
        self.z = z
        self.edge = is_edge

    def __str__(self):
        return f'x - {self.x}, y - {self.y}, z - {self.z} (is this an edge?: {self.edge}'
class Mesh:
    # sisendiks peab olema normaliseeritud dataset
    def __init__(self, dataset, minx, maxx, miny, maxy, sqrsize, naabrid, gridnum, voidcap=2):
        self.dataset = dataset
        self.minx = minx
        self.maxx = maxx
        self.miny = miny
        self.maxy = maxy
        self.sqrsize = sqrsize
        self.naabrid = naabrid  # mitu naabrit iga punkt peab leidma keskmise arvutamiseks minimaalselt
        self.gridnum = gridnum  # tuble ruutude arvu osas (x, y)
        self.voidcap = voidcap
        self.mesh = {}  #xcord{ycord}
        self.meshing()
        self.meshZcord = {}
        self.generategrid()
    def meshing(self):
        gridNx = int(self.gridnum[0])
        gridNy = int(self.gridnum[1])
        for i in range(gridNx+1):
            xdic = {}
            for j in range(gridNy+1):
                xdic.update({int(j): []})
            self.mesh.update({int(i): xdic})
        for pkt in self.dataset:
            xkord = int(pkt[0]//self.sqrsize)
            ykord = int(pkt[1]//self.sqrsize)
            self.mesh[xkord][ykord].append(pkt)
    def generateZcord(self, x, y):
        if (x == 0) or (x == self.gridnum[0]) or (y == 0) or (y == self.gridnum[1]):
            RN = Punkt(x, y, is_edge=True)
        else:
            RN = Punkt(x, y)
        if (0 <= (x - self.voidcap)) and ((x + self.voidcap) <= self.gridnum[0]):
            xkordmin = x - self.voidcap
            xkordmax = x + self.voidcap
        else:
            if (0 >= (x - self.voidcap)):
                xkordmin = 0
                xkordmax = x + self.voidcap
            elif ((x + self.voidcap) >= self.gridnum[0]):
                xkordmax = self.gridnum[0]
                xkordmin = x - self.voidcap
        if (0 <= (y - self.voidcap)) and ((y + self.voidcap) <= self.gridnum[1]):
            ykordmin = y - self.voidcap
            ykordmax = y + self.voidcap
        else:
            if (0 >= (y - self.voidcap)):
                ykordmin = 0
                ykordmax = y + self.voidcap
            elif ((y + self.voidcap) >= self.gridnum[1]):
                ykordmax = self.gridnum[1]
                ykordmin = x - self.voidcap
        i = xkordmin
        j = ykordmin
        dots = []
        vekts = []
        while i <= xkordmax:
            while j <= ykordmax:
                dots.extend(self.mesh[i][j])
        zval = 0.0
        zsum = 0.0
        znum = 0
        for dot in dots:
            RN = Vektor(x, dot[0], y, dot[1], z2= dot[2])
            vekts.append(RN)
        sortvekts = sorted(vekts, key=lambda vek: vek.pikk)
        while (znum < self.naabrid) or znum == len(vekts):
            zsum += sortvekts[znum].kaal
            zval += sortvekts[znum].kaal * sortvekts[znum].z1
            znum += 1
        RN.z = zval/zsum
        address = (x, y)
        self.meshZcord.update({address: RN})

    def generategrid(self):
        for xR in self.mesh.keys():
            for yR in self.mesh[xR].keys():
                self.generateZcord(xR, yR)
    def __str__(self):
        return f'Mesh x({self.minx}-{self.maxx}), y({self.miny}-{self.maxy}), milles on {len(self.dataset)} andmepunkti'


def csv2list(path=None):  # loeb csv faili ja tagastab listi punktide kordinaatidega
    if path == None:
        pathnimi = str(tkinter.filedialog.askopenfile())
        pathn = pathnimi.split("'")[1]
    data = []
    minx = float(0)
    maxx = float(0)
    miny = float(0)
    maxy = float(0)
    file = open(pathn, encoding='UTF-8')
    for rida in file:
        if rida != 'X,Y,Z\n':
            string = rida.strip('\n')
            RN = string.split(',')
            RNfl = [float(RN[0]), float(RN[1]), float(RN[2])]  # x,y,z
            if (minx == 0) or (minx >= RNfl[0]):  # leiame max ja min meshi jaoks
                minx = RNfl[0]
            if (maxx == 0) or (maxx <= RNfl[0]):
                maxx = RNfl[0]
            if (miny == 0) or (miny >= RNfl[1]):
                miny = RNfl[1]
            if (maxy == 0) or (maxy <= RNfl[1]):
                maxy = RNfl[1]
            data.append(RNfl)
    file.close()
    datanorm = []
    for pkt in data:
        RN = [(pkt[0]-minx), (pkt[1]-miny), pkt[2]]
        datanorm.append(RN)
    return datanorm, 0.0, maxx-minx, 0.0, maxy-miny

data, minx, maxx, miny, maxy = csv2list()
datasize = len(data)
meshx = maxx - minx
meshy = maxy - miny
idealtihedus = 4
naabrid = idealtihedus
sqrsize = 1
gridnum = (meshx/sqrsize, meshy/sqrsize)
m1 = Mesh(data, minx, maxx, miny, maxy, sqrsize, naabrid, gridnum)
for i in m1.meshZcord.keys():
    print(m1.meshZcord[i])



