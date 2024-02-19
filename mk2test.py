import math
import tkinter.filedialog


def arvutakaal(kaugusx, kaugusy):
    pikus = math.sqrt(((kaugusx) ** 2) + ((kaugusy) ** 2))
    if pikus == 0:
        # print(' null jagamine')
        return 100000000.0
    else:
        kaal = 1 / ((10 * pikus) ** 2)
        return kaal
class Syndot:
    def __init__(self, m, n):
        self.m = m #x cord
        self.n = n
        self.kaalutudnaabrid = {}  #{kaal: zcord of realdot}
        self.synZ = 0.0
        self.trueZ = False

    def arvutaZ(self):
        if not self.kaalutudnaabrid:
            self.synZ = 0.0
            self.trueZ = True
        else:
            zval = 0.0
            zsum = 0.0
            for dot in self.kaalutudnaabrid.keys():
                zval += self.kaalutudnaabrid[dot]*dot
                zsum += dot
            self.synZ = round(zval/zsum, 3)
            self.trueZ = True

    def __str__(self):
        if self.trueZ:
            return f'tehispukt kordinaatidel {self.m}, {self.n}, {self.synZ} - XYZ'
        else:
            return f'tehispukt kordinaatidel {self.m}, {self.n} XY. {len(self.kaalutudnaabrid.keys())} naabriga.'
class Realdot:
    def __init__(self, x, y, z, gridsize):
        self.x = x
        self.y = y
        self.z = z
        self.gridsize = gridsize
        self.sorteeritud = False

    def paigutasyndot(self, mastermesh):
        xkord = int(self.x//self.gridsize)
        ykord = int(self.y//self.gridsize)
        xmin = round((self.x % self.gridsize), 3)
        xmax = 1 - xmin
        ymin = round((self.y % self.gridsize), 3)
        ymax = 1 - ymin
        mastermesh.syndots[xkord][ykord].kaalutudnaabrid.update({arvutakaal(xmin, ymin): self.z})
        mastermesh.syndots[xkord + 1][ykord].kaalutudnaabrid.update({arvutakaal(xmax, ymin): self.z})
        mastermesh.syndots[xkord][ykord + 1].kaalutudnaabrid.update({arvutakaal(xmin, ymax): self.z})
        mastermesh.syndots[xkord + 1][ykord + 1].kaalutudnaabrid.update({arvutakaal(xmax, ymax): self.z})


        self.sorteeritud = True

    def __str__(self):
        return f'Andme punkt {self.x}, {self.y}, {self.z} XYZ, On paigutatud?: {self.sorteeritud}'

class Mesh:
    def __init__(self, data, sizex, sizey, sqrsize):
        self.data = data  #realdatapoints from lidar data NORMALIZED
        self.sizex = sizex  #number of xcord for syndata
        self.sizey = sizey  #number of ycord for syndata
        self.sqrsize = sqrsize  #sqaresize or distance from point to point
        self.syndots = {}  #dub dic of created syndata {x1: {x1y1: Syndot; x1y2: Syndot; ...}, x2:{...},...}
        self.createsyndotdic()
        self.sorteeridata()
        self.arvutaZsyndotidele()
    def createsyndotdic(self):  # tekitab ilma sisuta valmis synttetiliste andmepunktide ruudustiku
        i = 0  #x
        j = 0  #y
        while i <= self.sizex:
            dempdic = {}
            j = 0
            while j <= self.sizey:
                dempdic.update({j: Syndot(i, j)})
                j += 1
            self.syndots.update({i: dempdic})
            i += 1
        print('Synmesh created')
    def sorteeridata(self):  # tekitab realdot objektid ning koheselt paigutrab need syndot naabrite dictioneridesse
        for punkt in self.data:
            pktRN = Realdot(punkt[0], punkt[1], punkt[2], self.sqrsize)
            pktRN.paigutasyndot(self)
        print('Punktid on sorteeritud syndotidesse!')

    def arvutaZsyndotidele(self):
        for x in self.syndots.keys():
            for y in self.syndots.keys():
                self.syndots[x][y].arvutaZ()
        print('keksmine Z arvutud')
    def __str__(self):
        return f'mesh {self.sizex}, {self.sizey} - XY, andmepunkte on {len(self.data)}'


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
        RN = [round((pkt[0]-minx), 3), round((pkt[1]-miny), 3), round(pkt[2], 3)]
        datanorm.append(RN)
    return datanorm, 0.0, maxx-minx, 0.0, maxy-miny

data, minx, maxx, miny, maxy = csv2list()
datasize = len(data)
sqrsize = 1
sizex = math.ceil(maxx/sqrsize)
sizey = math.ceil(maxy/sqrsize)
m1 = Mesh(data, sizex, sizey, sqrsize)
