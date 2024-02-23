//
// Created by Jan Markus on 22/02/2024.
//

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <thread>
#include <map>
#include <tuple>

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
    public:
    std::map<std::tuple<long int>, Syndot> syndots; // tuple on kordinaatiderga (x, y) key et saada vastavad syndotinin
    double squresize;
    double sizex;
    double sizey;
    std::string nimi;
    /*create syndots jaetakse ara sest selle asemel saab genereerida syndotsid jooksvalt mapile
     * selleks et syndotilt saad kate andemd kasutatakse arvutaZ meetodit nins siis kustutatakse syndot mapilt koos keyga et saasta malu
     *
     */
};