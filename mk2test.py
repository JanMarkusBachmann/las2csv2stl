import math
import tkinter.filedialog
import datetime


def arvutakaal(kaugusx, kaugusy):
    pikus = math.sqrt(((kaugusx) ** 2) + ((kaugusy) ** 2))
    if pikus == 0:
        # print(' null jagamine')
        return 100000000.0
    else:
        kaal = 1 / ((10 * pikus) ** 2)
        return kaal

def leiaexpform(arv):
    if arv != 0:
        g = (math.log10(abs(arv)) // 1)
        e = arv / (10 ** g)
        if g < 0:
            mark = '-'
        else:
            mark = '+'
        if abs(g) < 10:
            exp = f'e{mark}00{round(abs(g))}'
        else:
            exp = f'e{mark}0{round(abs(g))}'
        return f'{round(e, 6)}{exp}'
    else:
        return '0.000000e+000'

class Triangle:
    vektorbank = {}  # globaalne bank arvutatud vektorite jaoks formaadis - [X, Y]: [a ->b]
    def __init__(self, a, b, c, type='up'):
        self.a = a  # punktid
        self.b = b
        self.c = c
        self.type = type
        self.ab = self.leiavektor(a, b)
        self.ac = self.leiavektor(a, c)
        self.bc = self.leiavektor(b, c)
        self.n = self.leianormaal(self.ab, self.ac, self.type)

    def __str__(self):
        return f'Kolmnurk {self.a.x}, {self.a.y}, {self.a.synZ} - XYZ punkt A'

    def leiavektor(self, a, b):
        if (a, b) in Triangle.vektorbank.keys():
            return Triangle.vektorbank[(a, b)]
        else:
            vx = b.x - a.x
            vy = b.y - a.y
            vz = b.synZ - a.synZ
            v = (vx, vy, vz)
            Triangle.vektorbank.update({(a, b): v})
            return v

    def leianormaal(self, a, b, tyyp='up'):
        xm = a[0]
        xn = b[0]
        ym = a[1]
        yn = b[1]
        zm = a[2]
        zn = b[2]
        # avrutame determinantidega vektor korrutise
        x = ym * zn - yn * zm
        y = -(xm * zn - xn * zm)
        z = xm * yn - ym * xn
        kordaja = math.sqrt(x**2 + y**2 + z**2)
        x = x/kordaja
        y = y/kordaja
        z = z/kordaja
        if tyyp == 'up':
            if z > 0:
                return (x, y, z)
            else:
                return (-x, -y, -z)
        elif tyyp == 'down':
            if z > 0:
                return (x, y, z)
            else:
                return (-x, -y, -z)

    def stlprintout(self):
        adot = f'vertex {leiaexpform(self.a.x)} {leiaexpform(self.a.y)} {leiaexpform(self.a.synZ)}'
        bdot = f'vertex {leiaexpform(self.b.x)} {leiaexpform(self.b.y)} {leiaexpform(self.b.synZ)}'
        cdot = f'vertex {leiaexpform(self.c.x)} {leiaexpform(self.c.y)} {leiaexpform(self.c.synZ)}'
        norm = f'facet normal {leiaexpform(self.n[0])} {leiaexpform(self.n[1])} {leiaexpform(self.n[2])}'
        return f'\t{norm}\n\t\touter loop\n\t\t\t{adot}\n\t\t\t{bdot}\n\t\t\t{cdot}\n\t\tendloop\n\tendfacet'



class Syndot:
    gridlen = 1.0
    def __init__(self, m, n):
        self.m = m  # x cord
        self.n = n
        self.kaalutudnaabrid = {}  # {kaal: zcord of realdot}
        self.synZ = 0.0
        self.trueZ = False
        self.x = Syndot.gridlen * self.m
        self.y = Syndot.gridlen * self.n

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
    def __init__(self, data, sizex, sizey, sqrsize, nimi):
        self.data = data  #realdatapoints from lidar data NORMALIZED
        self.sizex = sizex  #number of xcord for syndata
        self.sizey = sizey  #number of ycord for syndata
        self.sqrsize = sqrsize  #sqaresize or distance from point to point
        self.nimi = nimi
        Syndot.gridlen = self.sqrsize
        self.syndots = {}  #dub dic of created syndata {x1: {x1y1: Syndot; x1y2: Syndot; ...}, x2:{...},...}
        self.createsyndotdic()
        self.sorteeridata()
        self.data = []
        self.arvutaZsyndotidele()
        self.kolmnurgad = []
        self.genkolmnurad()
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
        print(f'Synmesh created {datetime.datetime.now()}')
    def sorteeridata(self):  # tekitab realdot objektid ning koheselt paigutrab need syndot naabrite dictioneridesse
        for punkt in self.data:
            pktRN = Realdot(punkt[0], punkt[1], punkt[2], self.sqrsize)
            pktRN.paigutasyndot(self)
        print(f'Punktid on sorteeritud syndotidesse! {datetime.datetime.now()}')

    def arvutaZsyndotidele(self):
        for x in self.syndots.keys():
            for y in self.syndots[x].keys():
                self.syndots[x][y].arvutaZ()
        print(f'keksmine z arvutud {datetime.datetime.now()}')

    def genkolmnurad(self):
        i = 0 #x
        while i < self.sizex:
            j = 0  #y
            while j < self.sizey:
                a = self.syndots[i][j]
                b = self.syndots[i + 1][j]
                c = self.syndots[i + 1][j + 1]
                d = self.syndots[i][j + 1]
                tri1 = Triangle(a, b, c)
                tri2 = Triangle(a, c, d)
                self.kolmnurgad.append(tri1)
                self.kolmnurgad.append(tri2)
                j += 1
            i += 1
        print(f'kolmnurgad genereeritud {datetime.datetime.now()}')

    def exportmesh(self):
        txt = []
        nimi = self.nimi
        txt.append(f'solid {nimi}')
        for tri in self.kolmnurgad:
            txt.append(tri.stlprintout())
        txt.append('endsolid')
        r = '\n'.join(txt)
        return r, nimi
    def __str__(self):
        return f'mesh {self.sizex}, {self.sizey} - XY, andmepunkte on {len(self.data)}'


def csv2list(path=None):  # loeb csv faili ja tagastab listi punktide kordinaatidega
    pathni = path
    if path == None:
        pathnimi = str(tkinter.filedialog.askopenfile())
        pathni = pathnimi.split("'")[1]
    print(datetime.datetime.now())
    data = []
    minx = float(0)
    maxx = float(0)
    miny = float(0)
    maxy = float(0)
    file = open(pathni, encoding='UTF-8')
    for rida in file:
        if rida != 'X,Y,Z\n' and rida != '\n':
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

nimi = input('mis on faili nimi?: ')
data, minx, maxx, miny, maxy = csv2list()
datasize = len(data)
sqrsize = 1
sizex = math.ceil(maxx/sqrsize)
sizey = math.ceil(maxy/sqrsize)
print(datetime.datetime.now())

m1 = Mesh(data, sizex, sizey, sqrsize, nimi)
mesh, nimi = m1.exportmesh()
f = open(f'{nimi}.stl', 'w', encoding='UTF-8')
print(datetime.datetime.now())
f.write(mesh)
print('valmis')