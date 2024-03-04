//
// Created by Jan Markus on 22/02/2024.
//
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <array>
#include <thread>
#include <map>
#include <tuple>
#include <cstdint>
#include <cstring>
#include <ctime>

double arvutakaal(float x, float y){
    double kaugus = std::sqrt(x*x + y*y);
    if (kaugus == 0.0) {
        return 10000;
    }
    double kaal = 1 / ((10 * kaugus) * (10 * kaugus));
    return kaal;
}

struct Point {
    float x, y, z;
};

class Vektor {
    public:
    float vx, vy, vz;
    Vektor(Point yks, Point kaks) {
        vx = kaks.x - yks.x;
        vy = kaks.y - yks.y;
        vz = kaks.z - yks.z;
    }
    Vektor(float _x, float _y, float _z) : vx(_x), vy(_y), vz(_z) {}
    Vektor(float x1, float x2, float y1, float y2, float z1, float z2){
        vx = x2 - x1;
        vy = y2 - y1;
        vz = z2 - z1;
    }
};

Vektor vektorkorutis(Vektor M, Vektor N){
    float x = M.vy * N.vz - N.vy * M.vz;

    float y = -(M.vx * N.vz - N.vx * M.vz);

    float z = M.vx * N.vy - M.vy * N.vx;

    float kordaja = std::sqrt(x*x + y*y + z*z);

    x = x/kordaja;
    y = y/kordaja;
    z = z/kordaja;

    if (z > 0) {
        return {x, y, z};
    }   else {
        return {-x, -y, -z};
    }
}

class Stltriangle {
        // plaan oleks constructoriga otse genereerida ja vb ka kirjutada kohe 50byte pack faili
        //buffer tuleb teha meetodiks mis selle valjastab
    private:
        Vektor normaal;
        Point a, b, c;
    public:
        //kordinaadid sisestatakse XYZ (paris kordinaadid mitte index) jarjekorras tuple'is ning nendega arvutatakse kolmnurga normaal
        Stltriangle(Point sa, Point sb, Point sc) : a(sa), b(sb), c(sc), normaal(vektorkorutis(Vektor(sa, sb), Vektor(sa, sc))) {};

        std::vector<uint8_t> bindataout() {
            std::vector<uint8_t> buffer(50);
            std::vector<float> data = {normaal.vx, normaal.vy, normaal.vz, a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z};
            std::memcpy(buffer.data(), data.data(), sizeof(float) * data.size());
            buffer[48] = 0;
            buffer[49] = 0;
            return buffer;
    }
};

class FileSTLbin {
public:
    std::vector<Stltriangle> kolmnurgad;

    void binwriteout(const std::string& nimi){
        //genereerimie failile nime ja binaar nime
        std::stringstream failinimi;
        failinimi << nimi << ".stl";
        std::string failn = failinimi.str();

        //avame faili
        std::ofstream outFile(failn, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open the file." << std::endl;}

        //valmistame headeri ja kirjutame selle faili
        std::vector<char> headerbuff(80);
        std::memcpy(headerbuff.data(), nimi.data(), nimi.size() * 8);
        for (auto i = (nimi.size()); i <= 80; i += 1){
            headerbuff[i-1] = 0;
        }
        outFile.write(reinterpret_cast<char*>(headerbuff.data()), 80);

        //kolmnurkade arv
        unsigned long int number = kolmnurgad.size();
        outFile.write(reinterpret_cast<char*>(&number), sizeof number);


        //kolmnurgad kirjutatakse sisse
        for (Stltriangle kolmnur : kolmnurgad) {
            outFile.write(reinterpret_cast<char*>(kolmnur.bindataout().data()), 50);
        }

        outFile.close();

    }
};

class Syndot {
    public:
        /* kaalutud naabrid asendatakse kahe summaga millele lisatakse realdot sorteerimise kaigus infot. zcordi arvutamisel alles arvutatakse kaalututd keskmine
        Lisaks ei pea syndot teadma oma enda kordinaate meshpinnal. Sellega on voimalik valtida malukulukat 4 doubel kordinaati */
        double zval = 0.0;
        double zsum = 0.0;
        double arvutaz() const {
            if (zsum != 0.0) {
                return zval/zsum;
            } else {std::cout << "syndot valmis aga datat pole \n";
            return 0.0;}
        };

};

class Mesh {
    private:
    float sqsize;
    long int lenx;
    long int leny;
    double zup;
    double zdown;
    std::string nimi;
    FileSTLbin stldat;
    /*create syndotsTihe jaetakse ara sest selle asemel saab genereerida syndotsid jooksvalt mapile
     * selleks et syndotilt saad kate andemd kasutatakse arvutaZ meetodit nins siis kustutatakse syndot mapilt koos keyga et saasta malu
     *
     */

    public:
    std::map<std::tuple<long int, long int>, Syndot> syndotsTihe; // tuple on kordinaatiderga (x, y) key et saada vastavad syndotinin

    std::vector<std::tuple<long int, long int>> XYvotmedTihe;

    std::map<std::tuple<long int, long int>, Point> PunktidXYZ; // tuple on kordinaatiderga (x, y) key et saada vastavad punktini

    std::vector<std::tuple<long int, long int>> baddots;



    void sqrmeshadd(float x, float y, float z){
        /* PYTHON code:
             * xkord = int(self.x//self.gridsize)
        ykord = int(self.y//self.gridsize)
        xmin = round((self.x % self.gridsize), 3)
        xmax = 1 - xmin
        ymin = round((self.y % self.gridsize), 3)
        ymax = 1 - ymin
        mastermesh.syndotsTihe[xkord][ykord].kaalutudnaabrid.update({arvutakaal(xmin, ymin): self.z})
        mastermesh.syndotsTihe[xkord + 1][ykord].kaalutudnaabrid.update({arvutakaal(xmax, ymin): self.z})
        mastermesh.syndotsTihe[xkord][ykord + 1].kaalutudnaabrid.update({arvutakaal(xmin, ymax): self.z})
        mastermesh.syndotsTihe[xkord + 1][ykord + 1].kaalutudnaabrid.update({arvutakaal(xmax, ymax): self.z})
             */
        float xdwn = std::fmod(x, sqsize);
        float xup = 1 - xdwn;
        float ydwn = std::fmod(y,sqsize);
        float yup = 1 - ydwn;

        std::tuple<long int, long int> xykey;
        auto xsec = static_cast<long int>(std::floor((x)/sqsize));
        auto ysec = static_cast<long int>(std::floor((y)/sqsize));
        std::get<0>(xykey) = xsec;
        std::get<1>(xykey) = ysec;

            //kontrollime kas punktid on olemas
        if (syndotsTihe.find(xykey) == syndotsTihe.end()) {
            XYvotmedTihe.push_back(xykey);
            syndotsTihe[xykey] = Syndot();
        }
        if (syndotsTihe.find({xsec + 1, ysec}) == syndotsTihe.end()) {
            XYvotmedTihe.emplace_back(xsec + 1, ysec);
            syndotsTihe[{xsec + 1, ysec}] = Syndot();
        }
        if (syndotsTihe.find({xsec, ysec + 1}) == syndotsTihe.end()) {
            XYvotmedTihe.emplace_back(xsec, ysec + 1);
            syndotsTihe[{xsec, ysec + 1}] = Syndot();
        }
        if (syndotsTihe.find({xsec + 1, ysec + 1}) == syndotsTihe.end()) {
            XYvotmedTihe.emplace_back(xsec + 1, ysec + 1);
            syndotsTihe[{xsec + 1, ysec + 1}] = Syndot();
        }
        //lisame igale naabrile vaartused
        syndotsTihe[xykey].zsum += arvutakaal(xdwn, ydwn);
        syndotsTihe[xykey].zval += arvutakaal(xdwn, ydwn) * z;
        syndotsTihe[{xsec + 1, ysec}].zsum += arvutakaal(xup, ydwn);
        syndotsTihe[{xsec + 1, ysec}].zval += arvutakaal(xup, ydwn) * z;
        syndotsTihe[{xsec, ysec + 1}].zsum += arvutakaal(xdwn, yup);
        syndotsTihe[{xsec, ysec + 1}].zval += arvutakaal(xdwn, yup) * z;
        syndotsTihe[{xsec + 1, ysec + 1}].zsum += arvutakaal(xup, yup);
        syndotsTihe[{xsec + 1, ysec + 1}].zval += arvutakaal(xup, yup) * z;
        //selle lahendusega on voimalik et koik syndotid ei genereerita. Selle jaoks peab kolmnurkade arvutamisel sellega arvestama
    }

    void syndata(long int start , unsigned long int end){
        // arvutame koigi syndotide vaartused
        for (int i = start; i <= end; i += 1){
            std::tuple<long, long> &voti = XYvotmedTihe[i];
            Point punktRN{};
            punktRN.x = static_cast<float>(std::get<0>(voti)) * sqsize;
            punktRN.y = static_cast<float>(std::get<1>(voti)) * sqsize;
            double z_double = syndotsTihe[voti].arvutaz(); // assuming arvutaz() returns double
            if (z_double > zup){
                std::cout << "Z liiga korge: " << punktRN.x << ", " << punktRN.y << ", " << z_double << "\n";}
            punktRN.z = static_cast<float>(z_double); // cast double to float
            PunktidXYZ[voti] = punktRN;
        }
    }

    void bruteforcedot(long int xcor, long int ycor){
        //see funktisoon saab argumendiks puudu oleva kordinaadi ning genereerib selle kordinaadid. kui ta ei leia kordinaaile rohkem kui 5 naabrit siis defineeritakse punkt 0 tasanile
        float parisnaabritearv = 0.0;
        float naabritesum = 0.0;
        int kaugusotsitav = 1;
        int x_1;
        int x_2;
        int y_1;
        int y_2;
        if (xcor - kaugusotsitav < 0) {
            x_1 = 0;
        } else {x_1 = xcor - kaugusotsitav;}

        if (xcor + kaugusotsitav >= lenx) {
            x_2 = lenx;
        } else {x_2 = xcor + kaugusotsitav;}

        if (ycor - kaugusotsitav < 0) {
            y_1 = 0;
        } else {y_1 = ycor - kaugusotsitav;}

        if (ycor + kaugusotsitav >= leny) {
            y_2 = leny;
        } else {y_2 = ycor + kaugusotsitav;}

        for (long int k = x_1; k <= x_2; k +=1){
            for (long int l = y_1; l <= y_2; l+=1) {
                if (PunktidXYZ.find({k, l}) != PunktidXYZ.end()) {
                    parisnaabritearv += 1;
                    naabritesum += PunktidXYZ[{k, l}].z;
                }
            }
        }

        float zRN = naabritesum/parisnaabritearv;
        Point punktRN{};
        punktRN.x = static_cast<float>(xcor) * sqsize;
        punktRN.y = static_cast<float>(ycor) * sqsize;
        if (parisnaabritearv >= 4){
        punktRN.z = zRN;
        if (punktRN.z > zup){
            std::cout << "Z liiga korge bruteforce error: " << punktRN.x << ", " << punktRN.y << ", " << zRN << "\n";}

        PunktidXYZ[{xcor, ycor}] = punktRN;} else {
            punktRN.z = 0.0;
            PunktidXYZ[{xcor, ycor}] = punktRN;
        }
        XYvotmedTihe.emplace_back(xcor, ycor);
    };

    void findbadDTS(){
        //otsime punkte mida mapis pole selleks et tegeleda nendega ning lahendada neile vaartused
        for (long int i = 0; i < lenx; i += 1){
            for (long int j = 0; j < leny; j += 1){
                if (syndotsTihe.find({i, j}) == syndotsTihe.end()) {
                    baddots.emplace_back(i, j);}
            }
        }
    };

    void forcedotseq(unsigned long int start, unsigned long int end){
        for (unsigned long int i = start; i < end; i += 1){
            long int RNx = std::get<0>(baddots[i]);
            long int RNy = std::get<1>(baddots[i]);
            bruteforcedot(RNx, RNy);
        }
    }

    void genereeriTRI(unsigned long int start, unsigned long int end){
        /*
         * i = 0 #x
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
         */
        for (auto i = start; i < end; i += 1) {
            long int RNx = std::get<0>(XYvotmedTihe[i]);
            long int RNy = std::get<1>(XYvotmedTihe[i]);
            if (RNx +1 >= lenx || RNy+1 >= leny) {
                continue;
            }
            Point a = PunktidXYZ[{RNx, RNy}];
            Point b = PunktidXYZ[{RNx + 1, RNy}];
            Point c = PunktidXYZ[{RNx, RNy + 1}];
            Point d = PunktidXYZ[{RNx + 1, RNy + 1}];
            stldat.kolmnurgad.emplace_back(a, b, d);
            stldat.kolmnurgad.emplace_back(a, d, c);
        } 
    };
    
    Mesh(std::string const& path, std::string const& meshnimi, float ruudusuurus) {
        std::ifstream file(path);
        sqsize = ruudusuurus;
        if (!file.is_open()) {
            throw std::invalid_argument("Error: Unable to open file!");

        }

        std::vector<std::vector<float>> data;
        float minx = 0.0, maxx = 0.0, miny = 0.0, maxy = 0.0;

        //Ricki mul on sinust vaga kahju aga me teeme erfiga normaliseeriva lahenudse et eemaldada data freakoutid
        float const zalfa = 2600.0; //zalfa2 95le = 1.96
        double sumZ2 = 0.0;
        double sumZ = 0.0;
        double znum = 0.0;

        std::string line;
        std::getline(file, line); // Read and ignore header

        while (std::getline(file, line)) {
            if (line.empty() || line == "X,Y,Z\n")
                continue;

            std::istringstream iss(line);
            std::string token;
            std::vector<float> coordinates;

            while (std::getline(iss, token, ',')) {
                coordinates.push_back(std::stof(token));
            }

            if (minx == 0 || minx >= coordinates[0])
                minx = coordinates[0];
            if (maxx == 0 || maxx <= coordinates[0])
                maxx = coordinates[0];
            if (miny == 0 || miny >= coordinates[1])
                miny = coordinates[1];
            if (maxy == 0 || maxy <= coordinates[1])
                maxy = coordinates[1];

            sumZ += coordinates[2];
            sumZ2 += coordinates[2]*coordinates[2];
            znum += 1;

            data.push_back(coordinates);
        }
        file.close();

        lenx = static_cast<long int>(std::floor((maxx - minx)/sqsize));
        leny = static_cast<long int>(std::floor((maxy - miny)/sqsize));

        double ex = sumZ/znum;
        double ex2 = sumZ2/znum;
        double dx = ex2 - (ex*ex);
        //arvutame usaldusintervallid

        zdown = ex - (zalfa * (dx / sqrt(znum)));
        zup = ex + (zalfa * (dx / sqrt(znum)));

        // kontrollime kas punkti vaartus on usutav ja kui ei ole siis eemdaldame andmestikust. zalfa vaartust voib muuta vajadusel
        for (const auto& point : data) {
            if (point[2] >= zdown && point[2] <= zup) {sqrmeshadd(point[0] - minx, point[1] - miny, point[2]);}
        }

        data.clear();
        std::cout << "andmed normaliseeritud ja sorteeritud";

        // valmistame kolmnurgad saadud dataga
        syndata(0, XYvotmedTihe.size()-1);

        //leiame koik syndotid mis jaid genereerimata sest datat pole voi on muu viga arvutustega
        findbadDTS();

        //for loop mida saaab paraleliseerida kus bruteforcitakse koik punktid
        forcedotseq(0, baddots.size());
        baddots.clear();
        syndotsTihe.clear();
        genereeriTRI(0, XYvotmedTihe.size()-1);
        stldat.binwriteout(meshnimi);
    }
};

std::string gettime(){
    std::time_t currentTime = std::time(nullptr);

    // Convert current time to string
    return std::ctime(&currentTime);
}

int main(){

    std::cout << gettime() << "\n";
    try {
        //Mesh m1(R"(C:\Users\Jan Markus\Documents\GitHub\las2csv2stl\data\testdata.csv)", "testSTlfile", 1.0);
        Mesh m1(R"(C:\Users\Jan Markus\Documents\GitHub\las2csv2stl\data\mastervana.csv)", "vanalinnsuurcpp", 0.5);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "\n" << gettime() << "\n";
    return 0;
}
