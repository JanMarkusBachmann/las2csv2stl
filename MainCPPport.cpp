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

double arvutakaal(float x, float y){
    double kaugus = std::sqrt(x*x + y*y);
    if (kaugus == 0.0) {
        return 10000;
    }
    double kaal = 1 / ((10 * kaugus) * (10 * kaugus));
    return kaal;
}

class Syndot {
    public:
        /* kaalutud naabrid asendatakse kahe summaga millele lisatakse realdot sorteerimise kaigus infot. zcordi arvutamisel alles arvutatakse kaalututd keskmine
        Lisaks ei pea syndot teadma oma enda kordinaate meshpinnal. Sellega on voimalik valtida malukulukat 4 doubel kordinaati */
        double zval = 0.0;
        double zsum = 0.0;
        double arvutaz(){
            if (zsum != 0.0) {
                return zval/zsum;
            }
        };

};
class Mesh {
    private:
    float sqsize;
    double lenx;
    double leny;
    std::string nimi;
    /*create syndotsTihe jaetakse ara sest selle asemel saab genereerida syndotsid jooksvalt mapile
     * selleks et syndotilt saad kate andemd kasutatakse arvutaZ meetodit nins siis kustutatakse syndot mapilt koos keyga et saasta malu
     *
     */
    public:
    std::map<std::tuple<long int, long int>, Syndot> syndotsTihe; // tuple on kordinaatiderga (x, y) key et saada vastavad syndotinin
    std::vector<std::tuple<long int, long int>> XYvotmedTihe;
    int genmesh(std::string const path = "C:\\Users\\Jan Markus\\Documents\\GitHub\\las2csv2stl\\data\\passa.csv") {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file!" << std::endl;
            return 1;
        }

        std::vector<std::vector<float>> data;
        float minx = 0.0f, maxx = 0.0f, miny = 0.0f, maxy = 0.0f;

        //Ricki mul on sinust vaga kahju aga me teeme erfiga normaliseeriva lahenudse et eemaldada data freakoutid
        float const zalfa95 = 1.96;
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

        lenx = maxx - minx;
        leny = maxy - miny;

        double ex = sumZ/znum;
        double ex2 = sumZ2/znum;
        double dx = ex2 - (ex*ex);
        //arvutame usaldusintervallid

        double usaldusZmin = ex - (zalfa95*(dx/ sqrt(znum)));
        double usaldusZmax = ex + (zalfa95*(dx/ sqrt(znum)));

        // kontrollime kas punkti vaartus on usutav ja kui ei ole siis eemdaldame andmestikust zalfa vaartust voib muuta vajadusel
        for (const auto& point : data) {
            if (point[2] <= usaldusZmin && point[2] >= usaldusZmax) {sqrmeshadd(point[0]-minx, point[1]-miny, point[2]);}
        }

        data.clear();
        return 0;
    }
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
    Mesh(std::string const pathnimi, std::string const meshnimi, float ruudusuurus) {
        sqsize = ruudusuurus;
        genmesh(pathnimi);

    }
};
