#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include<vector>
#include <sstream>
using namespace std;
#define BLACK_COLOR 0;
#define WHITE_COLOR 255;
/*
* Określa wyrównanie pakowania dla składowych struktury
* Wypycha bieżącą wartość wyrównania pakowania na wewnętrznym stosie kompilatora 
* i ustawia bieżącą wartość wyrównania pakowania na 2 - pakowanie do 2 bajtów
* Jest to potrzebne ze względu na typ „short”, po którym następuje typ „int”, będą 2 bajty do wypełnienia, więc int do 4 bajtów
*/
#pragma pack(push,2) 
//STRUKTURY//
//Struktura BITMAPFILEHEADER opisuje nagłówek pliku
struct BITMAPFILEHEADER {
    unsigned short bfType = { 0x4D42 };	//Sygnatura pliku, ‘BM’ dla prawidłowego pliku BMP
    unsigned int bfSize = 0;			//Długość całego pliku w bajtach
    unsigned short bfReserved1 = 0;		//Pole zarezerwowane1 (zwykle zero)
    unsigned short bfReserved2 = 0;		//Pole zarezerwowane2 (zwykle zero)
    unsigned int bfOffBits = 0;			//Pozycja danych obrazowych w pliku
}; 
//Struktura BITMAPINFOHEADER opisuje nagłówek obrazu
struct BITMAPINFOHEADER {
    unsigned int biSize = 0;			//Rozmiar nagłówka informacyjnego 
    unsigned int biWidth = 0;			//Szerokość obrazu w pikselach
    unsigned int biHeight = 0;			//Wysokość obrazu w pikselach
    unsigned short biPlanes = 1;		//Liczba płatów (musi być 1)
    unsigned short biBitCount = 0;		//Liczba bitów na piksel: 1, 4, 8, 16, 24 lub 32
    unsigned int biCompression = 0;		//Algorytm kompresji: BI_RGB=0, BI_RLE8=1,BI_RLE4 = 2, BI_BITFIELDS = 3
    unsigned int biSizeImage = 0;		//Rozmiar rysunku
    unsigned int biXpelsPerMeter = 0;	//Rozdzielczość pozioma
    unsigned int biYpelsPerMeter = 0;	//Rozdzielczość pionowa
    unsigned int biCrlUses = 0;			//Liczba kolorów w palecie
    unsigned int biCrlImportant = 0;	//Liczba ważnych kolorów w palecie
};
//Struktura Pixel24 - opisuje składowe pojedynczego pixela(R-red,G-Green,B-Blue)
struct Pixel24 {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};
//Usuwa rekord z góry wewnętrznego stosu kompilatora
#pragma pack(pop)
///////////////////////
//FUNKCJE - prototypy//
///////////////////////
string nazwa_pliku;     //zmienna przechowuje nazwe pliku do zapisu Bitmapy
int odczytajBFH(ifstream& ifs, BITMAPFILEHEADER& bfh);
int odczytajBIH(ifstream& ifs, BITMAPINFOHEADER& bih);
void PrintBFH(BITMAPFILEHEADER& bfh);
void PrintBIH(BITMAPINFOHEADER& bih);
vector<char> readBMPPixelData(string filename, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih);
vector<char> readBMPPixelDataPosision(string filename, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih, int h1, int h2);
void SaveBMPFile(ostream& os, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih, vector<char> data);
vector<char> sobelOperator(vector<char> data, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih);
vector<vector<int>> mask_load(const string& matrix_file);
vector<char> algorytm_custom(vector<char> inBuffer, int width, int height);
int rodzaj_maski();
int sprawdzczyint(string naturalna);
int pomoc();
void help1();
void help2();
//Glowna funkcja programu z MENU
int menu(BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih)
{
    //Plik BMP do odczytu
    ifstream plik;
    string nazwa,nazwa_pliku_do_zapisu, h_fragmentu1;
    int  h_fragmentu;
    //plik BMP do zapisu
    ofstream os;
    vector<char> data, data_out;	//Pixel data
    data.resize(bih.biSizeImage);
    while (true)
    {
        string wybor;
        system("cls");
        cout << "|------------------------------------------------------|" << endl;
        cout << "|                    PROGRAM GLOWNY                    |" << endl;
        cout << "|------------------------------------------------------|" << endl;
        cout << "|1 - Zaladowanie pliku BMP do programu                 |" << endl;
        cout << "|2 - Wyswietl informacje o bitmapie                    |" << endl;
        cout << "|3 - Wykonaj operacje Sobela na calym pliku            |" << endl;
        cout << "|4 - Wykonaj operacje wczytywania kolejnych fragmentow |" << endl;
        cout << "|    okreslonej wielosci                               |" << endl;
        cout << "|5 - Wykonaj operacje Sobela na maskach z pliku        |" << endl;
        cout << "|6 - Pomoc do programu                                 |" << endl;
        cout << "|7 - Zakoncz program                                   |" << endl;
        cout << "|------------------------------------------------------|" << endl;
        cout << "Wybierz opcje:";
        cin >> wybor;
        while (!sprawdzczyint(wybor))
        {
            cout << "Zly wybor, wprowadz jeszcze raz (Nie wprowadzono liczby)" << endl;
            cin >> wybor;
        }
        cout << endl;
        //string --> int
        switch (stoi(wybor))
        {
        case 1:
            cout << "Podaj nazwe pliku BMP do wczytania?" << endl;
            cin >> nazwa;
            plik.open(nazwa, ios::in);
            if (plik)
            {
                odczytajBFH(plik, bfh);
                odczytajBIH(plik, bih);
                if (bfh.bfType != 0x4d42) {
                    cout << "Podany plik nie jest plikiem BMP.";
                    this_thread::sleep_for(chrono::seconds(2));
                    plik.close();
                    plik.clear();
                    break;
                }
                //wykrywanie plików BMP, w ktorych biSizeImage=0
                if (bih.biSizeImage == 0) {
                    unsigned int scanLineSize = 4 * ((24 * bih.biWidth + 31) / 32);
                    unsigned int widthInBytes = sizeof(Pixel24) * bih.biWidth;
                    unsigned int paddingSize = scanLineSize - widthInBytes;
                    bih.biSizeImage = bih.biWidth * abs(int(bih.biHeight)) * 3 + bih.biHeight * paddingSize;
                    bih.biXpelsPerMeter = 0;
                    bih.biYpelsPerMeter = 0;
                }
            }
            else cout << "Nie uzyskano dostepu do pliku!" << endl;
            this_thread::sleep_for(chrono::seconds(2));
            plik.close();
            plik.clear();
            break;
        case 2:
            plik.open(nazwa, ios::in);
            if (plik)
            {
                system("cls");
                //Wyświetlenie parametrów pliku BMP
                odczytajBFH(plik, bfh);
                odczytajBIH(plik, bih);
                PrintBFH(bfh);
                PrintBIH(bih);
                system("pause");
            }
            else
            {
                cout << "Nie uzyskano dostepu do pliku!" << endl;
                this_thread::sleep_for(chrono::seconds(2));
            }
            plik.close();
            plik.clear();
            break;
        case 3:
            plik.open(nazwa, ios::in);
            cout << "Podaj nazwe pliku do zapisu(.bmp program doda sam): ";
            cin >> nazwa_pliku_do_zapisu;
            os.open((nazwa_pliku_do_zapisu+".bmp"), ios::binary);
            if (plik)
            {
                system("cls");
                data = readBMPPixelData(nazwa, bfh, bih);
                data_out.resize(bih.biWidth * bih.biHeight * 3);
                data_out = sobelOperator(data, bfh, bih);
                SaveBMPFile(os, bfh, bih, data_out);
                os.close();
                os.clear();
                cout << "Plik zapisano pod nazwa "<< nazwa_pliku_do_zapisu << ".bmp" << endl;
                this_thread::sleep_for(chrono::seconds(2));
                system("pause");
            }
            else
            {
                cout << "Nie uzyskano dostepu do pliku!" << endl;
                this_thread::sleep_for(chrono::seconds(2));
            }
            plik.close();
            plik.clear();
            break;
        case 4:
            plik.open(nazwa, ios::in);
            if (plik)
            {
                //załądowanie wszystkich danych pixeli do wektora data                               
                cout << "Podaj wysokosc fragmentu w px." << "Wysokosc obrazka oryginalnego= " << bih.biHeight << endl;
                cin >> h_fragmentu1;
                //sprawdznie czy wprowadzono intiger-a
                while (!sprawdzczyint(h_fragmentu1))
                {
                    cout << "Zly wybor, wprowadz jeszcze raz (Nie wprowadzono liczby)" << endl;
                    cin >> h_fragmentu1;
                }
                cout << endl;
                //string --> int
                h_fragmentu = stoi(h_fragmentu1);
                //sprawdzic czy h_fragmentu jest > od bih.biHeight - jezeli tak to ponownie wczytac
                while (h_fragmentu > bih.biHeight || h_fragmentu <=0)
                {
                    cout << "Rozmiar fragmentu niepoprawny, wprowadz ponownie" << endl;
                    cout << "Wprowadz ponownie:";
                    cin >> h_fragmentu;
                }
                cout << "Podaj nazwe pliku do zapisu(.bmp program doda sam): ";
                cin >> nazwa_pliku_do_zapisu;
                //liczba fragmentów o wysokości h_fragmentu
                int liczbafragmentow_h = bih.biHeight / h_fragmentu;
                //wysokość ostatniego fragmentu - moze byc 0
                int h_last = bih.biHeight - liczbafragmentow_h * h_fragmentu;
                cout << "Liczba fragmentow do przetworzenia=" << liczbafragmentow_h << " o wysokosci " << h_fragmentu << endl;
                cout << "Wysokosc ostatniego fragmentu=" << h_last << endl;
                int tmp = bih.biHeight;
                int h_calego_bufora = 0;
                //bufor calosci(polaczonych fragmentow)
                vector<char> FullPixelBuffer;
                //dla kazdego fragmentu o wysokości h_fragmentu tworzony jest bufor
                for (int fragment = 0; fragment <= liczbafragmentow_h - 1; fragment++) {
                    //bufor fragmentu
                    vector<char> partfileData;
                    partfileData.resize(h_fragmentu * bih.biWidth * 3);
                    cout << "Linie obrazu: " << (fragment * h_fragmentu) << "-" << ((fragment * h_fragmentu) + h_fragmentu - 1) << endl << endl;
                    partfileData = readBMPPixelDataPosision(nazwa, bfh, bih, (fragment * h_fragmentu), (fragment * h_fragmentu) + h_fragmentu - 1);
                    int tmp = bih.biHeight;
                    bih.biHeight = h_fragmentu;
                    partfileData = sobelOperator(partfileData, bfh, bih);
                    //dopisywanie
                    FullPixelBuffer.insert(FullPixelBuffer.end(), partfileData.begin(), partfileData.end());
                    //
                    int size = 0;
                    for (char i : FullPixelBuffer)
                        size = size + 1;
                    FullPixelBuffer.resize(size);
                    ofstream file_out2((nazwa_pliku_do_zapisu + ".bmp"), ios::binary);
                    bih.biHeight = bih.biHeight + h_fragmentu*fragment;
                    printf("H-----%d\n", bih.biHeight);
                    SaveBMPFile(file_out2, bfh, bih, FullPixelBuffer);
                    file_out2.close();
                    file_out2.clear();
                    h_calego_bufora = bih.biHeight;
                    //
                    bih.biHeight = tmp;
                }                
                if (h_last >= 1) {
                    vector<char> partfileData;
                    partfileData.resize(h_fragmentu * bih.biWidth * 3);
                    bih.biHeight = tmp;
                    cout << "Linie obrazu: "<< (bih.biHeight - h_last) << "-" << (bih.biHeight - 1) << endl << endl;
                    partfileData = readBMPPixelDataPosision(nazwa, bfh, bih, (bih.biHeight - h_last), (bih.biHeight - 1));
                    int tmp = bih.biHeight;
                    bih.biHeight = h_last;
                    partfileData = sobelOperator(partfileData, bfh, bih);
                    FullPixelBuffer.insert(FullPixelBuffer.end(), partfileData.begin(), partfileData.end());
                    //
                    int size = 0;
                    for (char i : FullPixelBuffer)
                        size = size + 1;
                    FullPixelBuffer.resize(size);
                    ofstream file_out2((nazwa_pliku_do_zapisu + ".bmp"), ios::binary);
                    bih.biHeight = h_calego_bufora + h_last;
                    printf("H-----%d\n", bih.biHeight);
                    SaveBMPFile(file_out2, bfh, bih, FullPixelBuffer);
                    file_out2.close();
                    file_out2.clear();
                    //
                    bih.biHeight = tmp;
                }
                cout << "Plik zapisano pod nazwa " << nazwa_pliku_do_zapisu << ".bmp" << endl;
                system("pause");
            }
            else
            {
                cout << "Nie uzyskano dostepu do pliku!" << endl;
                this_thread::sleep_for(chrono::seconds(2));
            }
            odczytajBFH(plik, bfh);
            odczytajBIH(plik, bih);
            plik.close();
            plik.clear();
            break;
        case 5:
            plik.open(nazwa, ios::in);
            if (plik)
            {
                system("cls");
                data = readBMPPixelData(nazwa, bfh, bih);
                data_out.resize(bih.biWidth * bih.biHeight * 3);
                data_out = algorytm_custom(data, bih.biWidth, bih.biHeight);
                nazwa_pliku.append("-");
                nazwa_pliku.append(nazwa);
                os.open(nazwa_pliku, ios::binary);
                cout << "Nazwa pliku do zapisu: " << nazwa_pliku.append(".bmp") << endl;
                SaveBMPFile(os, bfh, bih, data_out);
                os.close();
                os.clear();
            }
            else
            {
                cout << "Nie uzyskano dostepu do pliku!" << endl;
                this_thread::sleep_for(chrono::seconds(2));
            }
            plik.close();
            plik.clear();
            break;
        case 6:
            system("cls");
            pomoc();
            break;
        case 7:
            cout << "\n-------------------" << endl;
            cout << "--Do zobaczenia:)--" << endl;
            cout << "-------------------" << endl;
            this_thread::sleep_for(chrono::seconds(2));
            return 0;
            break;
        default:
            cout << "Zly wybor, wprowadz jeszcze raz" << endl;
            system("pause");
            break;
        }
    }
    return 0;
}
/*
*Funkcja zwraca wartosc odpowiadajaca rozmiarowi maski operator(3x3 - 33, 5x5 - 55, ....)
*/
int rodzaj_maski()
{
    int wybor;
    string wybor1;
    system("cls");
    cout << "|--------------|" << endl;
    cout << "| Rodzaj Maski |" << endl;
    cout << "|--------------|" << endl;
    cout << "|33 - 3x3      |" << endl;
    cout << "|55 - 5x5      |" << endl;
    cout << "|77 - 7x7      |" << endl;
    cout << "|99 - 9x9      |" << endl;
    cout << "|--------------|" << endl;
    cout << "Wybierz opcje:";
    cin >> wybor1;

    while (!sprawdzczyint(wybor1))
    {
        cout << "Zly wybor, wprowadz jeszcze raz (Nie wprowadzono liczby)" << endl;
        cin >> wybor1;
    }
    wybor = stoi(wybor1);
    //sprawdzić czy wybor jest wlasciwy 
    while (wybor!=33 && wybor != 55 && wybor != 77 && wybor != 99)
    {
        cout << "Nie ma takiej opcji" << endl;
        cout << "Wybierz ponownie rodzaj maski" << endl;
        cin.clear();
        cin.ignore();
        cin >> wybor;
    }
    return wybor;
}
/*
*Funkcja zwraca liczbe masek przez ktora bedzie dokonywana operacja konwolucji
*/
int liczba_masek() 
{
    string wybor;
    system("cls");
    cout << "Podaj liczbe masek do operacji (1-8): ";
    cin >> wybor;
    while (!sprawdzczyint(wybor))
    {
        cout << "Zly wybor, wprowadz jeszcze raz (Nie wprowadzono liczby)" << endl;
        cin >> wybor;
    }
    int wybor1 = stoi(wybor);
    while (wybor1 < 1 || wybor1 > 8)
    {
        cout << "Masek jest od 1 do 8" << endl;
        cout << "Wprowadz ponownie liczbe masek" << endl;
        cin.clear();
        cin.ignore();
        cin >> wybor1;
    } 
    return wybor1;
}
/*
Funkcja odczytuje bufor obrazu(tylko pixele) z wektora inBuffer, zapisuje wartości pixeli po
odpowiedniej operacji konwulucji w wektorze wyjsciowym. Operacja konwulucji jest realizowana na
wielkosci obrazu w pixelach o wymiarach width x height i odpwiedniej ilosci masek i rodzaju.
Rodzaj masek i ich ilosc jest podana jako zapytanie.
*/
vector<char> algorytm_custom(vector<char> inBuffer, int width, int height)
{
    cout << "Podaj rodzaj maski: ";
    int jaka_maska = rodzaj_maski();
    int liczba_masek_do_operacji = liczba_masek();
    int szerokosc_ramki_w_px = 0;
    vector<vector<int>> sobel_S1_3x3, sobel_S2_3x3, sobel_S3_3x3, sobel_S4_3x3,
        sobel_S5_3x3, sobel_S6_3x3, sobel_S7_3x3, sobel_S8_3x3;
    vector<vector<int>> sobel_S1_5x5, sobel_S2_5x5, sobel_S3_5x5, sobel_S4_5x5,
        sobel_S5_5x5, sobel_S6_5x5, sobel_S7_5x5, sobel_S8_5x5;
    vector<vector<int>> sobel_S1_7x7, sobel_S2_7x7, sobel_S3_7x7, sobel_S4_7x7,
        sobel_S5_7x7, sobel_S6_7x7, sobel_S7_7x7, sobel_S8_7x7;
    vector<vector<int>> sobel_S1_9x9, sobel_S2_9x9, sobel_S3_9x9, sobel_S4_9x9,
        sobel_S5_9x9, sobel_S6_9x9, sobel_S7_9x9, sobel_S8_9x9;
    switch (jaka_maska)
    {
    case 33: //maska 3x3 
        switch (liczba_masek_do_operacji)
        {
        case 1:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            break;
        case 2:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            break;
        case 3:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            sobel_S3_3x3 = mask_load(".//3x3//Sobel_S3_3x3.txt");
            break;
        case 4:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            sobel_S3_3x3 = mask_load(".//3x3//Sobel_S3_3x3.txt");
            sobel_S4_3x3 = mask_load(".//3x3//Sobel_S4_3x3.txt");
            break;
        case 5:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            sobel_S3_3x3 = mask_load(".//3x3//Sobel_S3_3x3.txt");
            sobel_S4_3x3 = mask_load(".//3x3//Sobel_S4_3x3.txt");
            sobel_S5_3x3 = mask_load(".//3x3//Sobel_S5_3x3.txt");
            break;
        case 6:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            sobel_S3_3x3 = mask_load(".//3x3//Sobel_S3_3x3.txt");
            sobel_S4_3x3 = mask_load(".//3x3//Sobel_S4_3x3.txt");
            sobel_S5_3x3 = mask_load(".//3x3//Sobel_S5_3x3.txt");
            sobel_S6_3x3 = mask_load(".//3x3//Sobel_S6_3x3.txt");
            break;
        case 7:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            sobel_S3_3x3 = mask_load(".//3x3//Sobel_S3_3x3.txt");
            sobel_S4_3x3 = mask_load(".//3x3//Sobel_S4_3x3.txt");
            sobel_S5_3x3 = mask_load(".//3x3//Sobel_S5_3x3.txt");
            sobel_S6_3x3 = mask_load(".//3x3//Sobel_S6_3x3.txt");
            sobel_S7_3x3 = mask_load(".//3x3//Sobel_S7_3x3.txt");
            break;
        case 8:
            sobel_S1_3x3 = mask_load(".//3x3//Sobel_S1_3x3.txt");
            sobel_S2_3x3 = mask_load(".//3x3//Sobel_S2_3x3.txt");
            sobel_S3_3x3 = mask_load(".//3x3//Sobel_S3_3x3.txt");
            sobel_S4_3x3 = mask_load(".//3x3//Sobel_S4_3x3.txt");
            sobel_S5_3x3 = mask_load(".//3x3//Sobel_S5_3x3.txt");
            sobel_S6_3x3 = mask_load(".//3x3//Sobel_S6_3x3.txt");
            sobel_S7_3x3 = mask_load(".//3x3//Sobel_S7_3x3.txt");
            sobel_S8_3x3 = mask_load(".//3x3//Sobel_S8_3x3.txt");
            break;
        default:
            cout << "Do wyboru jest od 1 do 8 masek" << endl;
            system("pause");
            break;
        }
            szerokosc_ramki_w_px = 1;
            nazwa_pliku = "output_3x3_";
            break;
    case 55: //maska 5x5
        switch (liczba_masek_do_operacji)
        {
        case 1:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            break;
        case 2:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            break;
        case 3:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            sobel_S3_5x5 = mask_load(".//5x5//Sobel_S3_5x5.txt");
            break;
        case 4:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            sobel_S3_5x5 = mask_load(".//5x5//Sobel_S3_5x5.txt");
            sobel_S4_5x5 = mask_load(".//5x5//Sobel_S4_5x5.txt");
            break;
        case 5:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            sobel_S3_5x5 = mask_load(".//5x5//Sobel_S3_5x5.txt");
            sobel_S4_5x5 = mask_load(".//5x5//Sobel_S4_5x5.txt");
            sobel_S5_5x5 = mask_load(".//5x5//Sobel_S5_5x5.txt");
            break;
        case 6:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            sobel_S3_5x5 = mask_load(".//5x5//Sobel_S3_5x5.txt");
            sobel_S4_5x5 = mask_load(".//5x5//Sobel_S4_5x5.txt");
            sobel_S5_5x5 = mask_load(".//5x5//Sobel_S5_5x5.txt");
            sobel_S6_5x5 = mask_load(".//5x5//Sobel_S6_5x5.txt");
            break;
        case 7:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            sobel_S3_5x5 = mask_load(".//5x5//Sobel_S3_5x5.txt");
            sobel_S4_5x5 = mask_load(".//5x5//Sobel_S4_5x5.txt");
            sobel_S5_5x5 = mask_load(".//5x5//Sobel_S5_5x5.txt");
            sobel_S6_5x5 = mask_load(".//5x5//Sobel_S6_5x5.txt");
            sobel_S7_5x5 = mask_load(".//5x5//Sobel_S7_5x5.txt");
            break;
        case 8:
            sobel_S1_5x5 = mask_load(".//5x5//Sobel_S1_5x5.txt");
            sobel_S2_5x5 = mask_load(".//5x5//Sobel_S2_5x5.txt");
            sobel_S3_5x5 = mask_load(".//5x5//Sobel_S3_5x5.txt");
            sobel_S4_5x5 = mask_load(".//5x5//Sobel_S4_5x5.txt");
            sobel_S5_5x5 = mask_load(".//5x5//Sobel_S5_5x5.txt");
            sobel_S6_5x5 = mask_load(".//5x5//Sobel_S6_5x5.txt");
            sobel_S7_5x5 = mask_load(".//5x5//Sobel_S7_5x5.txt");
            sobel_S8_5x5 = mask_load(".//5x5//Sobel_S8_5x5.txt");
            break;
        default:
            cout << "Do wyboru jest od 1 do 8 masek" << endl;
            system("pause");
            break;
        }
            szerokosc_ramki_w_px = 3;
            nazwa_pliku = "output_5x5_";
            break;
    case 77: //maska 7x7
        switch (liczba_masek_do_operacji)
        {
        case 1:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            break;
        case 2:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            break;
        case 3:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            sobel_S3_7x7 = mask_load(".//7x7//Sobel_S3_7x7.txt");
            break;
        case 4:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            sobel_S3_7x7 = mask_load(".//7x7//Sobel_S3_7x7.txt");
            sobel_S4_7x7 = mask_load(".//7x7//Sobel_S4_7x7.txt");
            break;
        case 5:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            sobel_S3_7x7 = mask_load(".//7x7//Sobel_S3_7x7.txt");
            sobel_S4_7x7 = mask_load(".//7x7//Sobel_S4_7x7.txt");
            sobel_S5_7x7 = mask_load(".//7x7//Sobel_S5_7x7.txt");
            break;
        case 6:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            sobel_S3_7x7 = mask_load(".//7x7//Sobel_S3_7x7.txt");
            sobel_S4_7x7 = mask_load(".//7x7//Sobel_S4_7x7.txt");
            sobel_S5_7x7 = mask_load(".//7x7//Sobel_S5_7x7.txt");
            sobel_S6_7x7 = mask_load(".//7x7//Sobel_S6_7x7.txt");
            break;
        case 7:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            sobel_S3_7x7 = mask_load(".//7x7//Sobel_S3_7x7.txt");
            sobel_S4_7x7 = mask_load(".//7x7//Sobel_S4_7x7.txt");
            sobel_S5_7x7 = mask_load(".//7x7//Sobel_S5_7x7.txt");
            sobel_S6_7x7 = mask_load(".//7x7//Sobel_S6_7x7.txt");
            sobel_S7_7x7 = mask_load(".//7x7//Sobel_S7_7x7.txt");
            break;
        case 8:
            sobel_S1_7x7 = mask_load(".//7x7//Sobel_S1_7x7.txt");
            sobel_S2_7x7 = mask_load(".//7x7//Sobel_S2_7x7.txt");
            sobel_S3_7x7 = mask_load(".//7x7//Sobel_S3_7x7.txt");
            sobel_S4_7x7 = mask_load(".//7x7//Sobel_S4_7x7.txt");
            sobel_S5_7x7 = mask_load(".//7x7//Sobel_S5_7x7.txt");
            sobel_S6_7x7 = mask_load(".//7x7//Sobel_S6_7x7.txt");
            sobel_S7_7x7 = mask_load(".//7x7//Sobel_S7_7x7.txt");
            sobel_S8_7x7 = mask_load(".//7x7//Sobel_S8_7x7.txt");
            break;
        default:
            cout << "Do wyboru jest od 1 do 8 masek" << endl;
            system("pause");
            break;
        }
            szerokosc_ramki_w_px = 5;
            nazwa_pliku = "output_7x7_";
            break;
    case 99: //maska 9x9
        switch (liczba_masek_do_operacji)
        {
        case 1:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            break;
        case 2:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            break;
        case 3:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            sobel_S3_9x9 = mask_load(".//9x9//Sobel_S3_9x9.txt");
            break;
        case 4:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            sobel_S3_9x9 = mask_load(".//9x9//Sobel_S3_9x9.txt");
            sobel_S4_9x9 = mask_load(".//9x9//Sobel_S4_9x9.txt");
            break;
        case 5:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            sobel_S3_9x9 = mask_load(".//9x9//Sobel_S3_9x9.txt");
            sobel_S4_9x9 = mask_load(".//9x9//Sobel_S4_9x9.txt");
            sobel_S5_9x9 = mask_load(".//9x9//Sobel_S5_9x9.txt");
            break;
        case 6:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            sobel_S3_9x9 = mask_load(".//9x9//Sobel_S3_9x9.txt");
            sobel_S4_9x9 = mask_load(".//9x9//Sobel_S4_9x9.txt");
            sobel_S5_9x9 = mask_load(".//9x9//Sobel_S5_9x9.txt");
            sobel_S6_9x9 = mask_load(".//9x9//Sobel_S6_9x9.txt");
            break;
        case 7:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            sobel_S3_9x9 = mask_load(".//9x9//Sobel_S3_9x9.txt");
            sobel_S4_9x9 = mask_load(".//9x9//Sobel_S4_9x9.txt");
            sobel_S5_9x9 = mask_load(".//9x9//Sobel_S5_9x9.txt");
            sobel_S6_9x9 = mask_load(".//9x9//Sobel_S6_9x9.txt");
            sobel_S7_9x9 = mask_load(".//9x9//Sobel_S7_9x9.txt");
            break;
        case 8:
            sobel_S1_9x9 = mask_load(".//9x9//Sobel_S1_9x9.txt");
            sobel_S2_9x9 = mask_load(".//9x9//Sobel_S2_9x9.txt");
            sobel_S3_9x9 = mask_load(".//9x9//Sobel_S3_9x9.txt");
            sobel_S4_9x9 = mask_load(".//9x9//Sobel_S4_9x9.txt");
            sobel_S5_9x9 = mask_load(".//9x9//Sobel_S5_9x9.txt");
            sobel_S6_9x9 = mask_load(".//9x9//Sobel_S6_9x9.txt");
            sobel_S7_9x9 = mask_load(".//9x9//Sobel_S7_9x9.txt");
            sobel_S8_9x9 = mask_load(".//9x9//Sobel_S8_9x9.txt");
            break;
        default:
            cout << "Do wyboru jest od 1 do 8 masek" << endl;
            system("pause");
            break;
        }
            szerokosc_ramki_w_px = 7;
            nazwa_pliku = "output_9x9_";
            break;
     default:
     cout << "Nie ma takiej maski" << endl;
     system("pause");
     break;
    }
    cout << "Liczba masek = " << liczba_masek_do_operacji << "\n" << "Rodzaj maski = " << jaka_maska << "\n" << endl;
    string str;
    stringstream ss;
    ss << liczba_masek_do_operacji; ss >> str;
    str.append("masek");
    nazwa_pliku = nazwa_pliku.append(str);
    inBuffer.resize(width * height * 3);
    vector<char> outBuffer;
    outBuffer.resize(width * height * 3);
    Pixel24** image = new Pixel24 * [height];
    Pixel24** image_new = new Pixel24 * [height];
    for (unsigned int i = 0; i < height; i++)
    {
        image[i] = new Pixel24[width]; //przydzielenie dla każdego wiersza po i komórek
        image_new[i] = new Pixel24[width];
    }
    int liczba_pixeli_w_linii = 3 * width;
    int ibRow = 0;
    int ibPixel = 0;
    for (unsigned int y = 0; y < height; y = y + 1) {
        ibPixel = ibRow;
        for (unsigned int x = 0; x < width; x = x + 1) {
            image[y][x].R = inBuffer[ibPixel + 2];
            image[y][x].G = inBuffer[ibPixel + 1];
            image[y][x].B = inBuffer[ibPixel + 0];
            ibPixel = ibPixel + 3;
        }
        ibRow = ibRow + liczba_pixeli_w_linii;
    }
    printf("\nAlgorytm Start...\n");
    //algorytm///
    for (unsigned int lin = 0; lin < height; lin++) {
        for (unsigned int col = 0; col < width; col++) {
            if (lin >= szerokosc_ramki_w_px && lin < height - szerokosc_ramki_w_px - 1 && col >= szerokosc_ramki_w_px && col < width - szerokosc_ramki_w_px)
            {
                int ValBlueS1 = 0, ValBlueS2 = 0, ValBlueS3 = 0, ValBlueS4 = 0, ValBlueS5 = 0, ValBlueS6 = 0, ValBlueS7 = 0, ValBlueS8 = 0;
                int ValGreenS1 = 0, ValGreenS2 = 0, ValGreenS3 = 0, ValGreenS4 = 0, ValGreenS5 = 0, ValGreenS6 = 0, ValGreenS7 = 0, ValGreenS8 = 0;
                int ValRedS1 = 0, ValRedS2 = 0, ValRedS3 = 0, ValRedS4 = 0, ValRedS5 = 0, ValRedS6 = 0, ValRedS7 = 0, ValRedS8 = 0;
                //przetwarzanie obrazu
                int matrix_size = 1;
                //tworzenie nazwy pliku do zapisu dla funkcji custom algorytm
                switch (jaka_maska)
                {
                case 33:
                    matrix_size = 1;
                    break;
                case 55:
                    matrix_size = 2;
                    break;
                case 77:
                    matrix_size = 3;
                    break;
                case 99:
                    matrix_size = 4;
                    break;
                }
                for (unsigned int x = lin - matrix_size; x <= lin + matrix_size; x++)
                {
                    for (unsigned int y = col - matrix_size; y <= col + matrix_size; y++)
                    {
                        switch (jaka_maska)
                        {
                        case 33:
                            switch (liczba_masek_do_operacji)
                            {
                            case 1:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 2:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 3:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 4:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 5:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 6:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 7:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 8:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS8 += (image[x][y].B * sobel_S8_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS8 += (image[x][y].G * sobel_S8_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS8 += (image[x][y].R * sobel_S8_3x3[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            default:
                                break;
                            }
                            break;
                        case 55:
                            switch (liczba_masek_do_operacji)
                            {
                            case 1:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 2:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 3:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 4:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 5:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 6:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 7:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 8:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS8 += (image[x][y].B * sobel_S8_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS8 += (image[x][y].G * sobel_S8_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS8 += (image[x][y].R * sobel_S8_5x5[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            default:
                                break;
                            }
                            break;
                        case 77:
                            switch (liczba_masek_do_operacji)
                            {
                            case 1:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 2:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 3:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 4:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 5:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 6:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 7:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 8:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS8 += (image[x][y].B * sobel_S8_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS8 += (image[x][y].G * sobel_S8_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS8 += (image[x][y].R * sobel_S8_7x7[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            default:
                                break;
                            }
                            break;
                        case 99:
                            switch (liczba_masek_do_operacji)
                            {
                            case 1:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 2:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 3:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 4:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 5:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 6:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 7:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            case 8:
                                //Pixel Blue - Konwolucja dla składowej B
                                ValBlueS1 += (image[x][y].B * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS2 += (image[x][y].B * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS3 += (image[x][y].B * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS4 += (image[x][y].B * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS5 += (image[x][y].B * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS6 += (image[x][y].B * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS7 += (image[x][y].B * sobel_S7_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValBlueS8 += (image[x][y].B * sobel_S8_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Green - Konwolucja dla składowej G
                                ValGreenS1 += (image[x][y].G * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS2 += (image[x][y].G * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS3 += (image[x][y].G * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS4 += (image[x][y].G * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS5 += (image[x][y].G * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS6 += (image[x][y].G * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS7 += (image[x][y].G * sobel_S7_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValGreenS8 += (image[x][y].G * sobel_S8_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                //Pixel Red - Konwolucja dla składowej R
                                ValRedS1 += (image[x][y].R * sobel_S1_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS2 += (image[x][y].R * sobel_S2_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS3 += (image[x][y].R * sobel_S3_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS4 += (image[x][y].R * sobel_S4_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS5 += (image[x][y].R * sobel_S5_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS6 += (image[x][y].R * sobel_S6_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS7 += (image[x][y].R * sobel_S7_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                ValRedS8 += (image[x][y].R * sobel_S8_9x9[x - lin + matrix_size][y - col + matrix_size]);
                                break;
                            default:
                                break;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
                int Blue = (int)(abs(ValBlueS1) + abs(ValBlueS2) + abs(ValBlueS3) + abs(ValBlueS4) + abs(ValBlueS5) + abs(ValBlueS6) + abs(ValBlueS7) + abs(ValBlueS8)) / 8;
                int Green = (int)(abs(ValGreenS1) + abs(ValGreenS2) + abs(ValGreenS3) + abs(ValGreenS4) + abs(ValGreenS5) + abs(ValGreenS6) + abs(ValGreenS7) + abs(ValGreenS8)) / 8;
                int Red = (int)(abs(ValRedS1) + abs(ValRedS2) + abs(ValRedS3) + abs(ValRedS4) + abs(ValRedS5) + abs(ValRedS6) + abs(ValRedS7) + abs(ValRedS8)) / 8;
                if (Blue > 255)
                {
                    image_new[lin][col].B = WHITE_COLOR;
                }
                else {
                    image_new[lin][col].B = Blue;
                }
                if (Green > 255)
                {
                    image_new[lin][col].G = WHITE_COLOR;
                }
                else {
                    image_new[lin][col].G = Green;
                }
                if (Red > 255)
                {
                    image_new[lin][col].R = WHITE_COLOR;
                }
                else {
                    image_new[lin][col].R = Red;
                }
            }
            else
            {
                //nadanie koloru ramce dla której nie możliwe jest obliczenie wartosci pixela
                image_new[lin][col].B = BLACK_COLOR;
                image_new[lin][col].G = BLACK_COLOR;
                image_new[lin][col].R = BLACK_COLOR;
            }
        }
    }
    printf("\nAlgorytm Stop...\n");
    //////////////////////////////////////////////////////////
    ibRow = 0;
    ibPixel = 0;
    for (unsigned int y = 0; y < height; y = y + 1) {
        ibPixel = ibRow;
        for (unsigned int x = 0; x < width; x = x + 1) {
            outBuffer[(double)ibPixel + (double)0] = image_new[y][x].B;
            outBuffer[(double)ibPixel + (double)1] = image_new[y][x].G;
            outBuffer[(double)ibPixel + (double)2] = image_new[y][x].R;
            ibPixel = ibPixel + 3;
        }
        ibRow = ibRow + liczba_pixeli_w_linii;
    }
    //zwolnienie pamieci
    for (unsigned int i = 0; i < height; i++)
    {
        delete[] image[i];
        delete[] image_new[i];
    }
    delete[] image;
    delete[] image_new;
    return outBuffer;
}
/*
* Funkcja zwraca wektor maski. Jako parametr podaje sie nazwe pliku maski.
* maska_S3_5x5.txt
*        |  |
*        |  oznacza operator Sobela o wymiarach 5x5(dostepne opcje 3x3,5x5,7x7,9x9)
*        oznaczas maskę obrocona o 90st - krawedzie poziome
*        S1-0st     -   krawedzie pionowe
*        S2-45st    -   krawedzie ukosne
*        S3-90st    -   krawedzie poziome
*        S4-135st   -   krawedzie ukosne
*        S5-180st   -   =   -S1
*        S6-225st   -   =   -S2
*        S7-270st   -   =   -S3
*        S8-315st   -   =   -S4
*/
vector<vector<int>> mask_load(const string& matrix_file)
{
    int row, col;
    cout << "	<-	Ladowanie pliku maski: " << matrix_file << endl;
    vector<vector<int>> matrix;
    ifstream file(matrix_file);
    if (!file)
    {
        cerr << "Blad otwarcia pliku.\n";
    }
    file >> row >> col;
    if (row < 1 || col < 1)
    {
        cerr << "Rozmiar maski niepoprawny.\n";
    }
    matrix.resize(row);
    for (auto& m : matrix)
        m.resize(col);
    // Odczyt z pliku
    for (auto& outer : matrix)
        for (auto& inner : outer)
            file >> inner;
    return matrix;
}
/*
*Funkcja odczytuje bufor obrazu(tylko pixele) z wektora data, zapisuje wartości pixeli po
*operacji konwulucji w wektorze wyjsciowym. Operacja konwulucji jest realizowana na
*wielkosci obrazu o wymiarach bih.biWidth x bih.biHeight dla 8 masek(S1-S8) o wymiarach 3x3.
*Dane bih.biWidth i bih.biHeight są odczytane ze struktur BITMAPFILEHEADER i BITMAPINFOHEADER
*/
vector<char> sobelOperator(vector<char> data, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih)
{
    //operatory Sobela
    int sobel_S1[3][3] =
    {
      { -1, 0, 1 },
      { -2, 0, 2 },
      { -1, 0, 1 }
    };
    int sobel_S2[3][3] =
    {
      {  0,  1,  2 },
      { -1,  0,  1 },
      { -2, -1,  0 }
    };
    int sobel_S3[3][3] =
    {
      {  1,  2,  1 },
      {  0,  0,  0 },
      { -1, -2, -1 }
    };
    int sobel_S4[3][3] =
    {
      {  2,  1,  0 },
      {  1,  0, -1 },
      {  0, -1, -2 }
    };
    int sobel_S5[3][3] =
    {
      { 1, 0, -1 },
      { 2, 0, -2 },
      { 1, 0, -1 }
    };
    int sobel_S6[3][3] =
    {
      {  0, -1, -2 },
      {  1,  0, -1 },
      {  2,  1,  0 }
    };
    int sobel_S7[3][3] =
    {
      { -1, -2, -1 },
      {  0,  0,  0 },
      {  1,  2,  1 }
    };
    int sobel_S8[3][3] =
    {
      { -2, -1, 0 },
      { -1,  0, 1 },
      {  0,  1, 2 }
    };
    //Definicja wektora do zapsania przetworzonych Pixeli obrazu
    vector<char> fileData;
    //ustalenie wielkosci wektora
    fileData.resize(bih.biSizeImage);
    //utworzenie tablicy dynamicznej 2 wymiarowej o wielkosci bih.biHeight X bih.biWidth
    //Kazdy element tej tablicy jest typu Pixel24, czyli ma 3 skladowe R/G/B
    Pixel24** image = new Pixel24 * [bih.biHeight];
    Pixel24** image_new = new Pixel24 * [bih.biHeight];
    for (unsigned int i = 0; i < bih.biHeight; i++)
    {
        image[i] = new Pixel24[bih.biWidth]; //przydzielenie dla każdego wiersza po i komórek
        image_new[i] = new Pixel24[bih.biWidth];
    }
    int liczba_pixeli_w_linii = 3 * bih.biWidth;
    int ibRow = 0;
    size_t ibPixel = 0;
    //przepisanie danych obrazu z wektora to tablicy dynamicznej image[][]
    //na tej tablicy zostanie przeprowadzony algorytm konwulucji dla 8 operatorow Sobela
    for (unsigned int y = 0; y < bih.biHeight; y = y + 1) {
        ibPixel = ibRow;
        for (unsigned int x = 0; x < bih.biWidth; x = x + 1) {
            image[y][x].R = data[ibPixel + 2];
            image[y][x].G = data[ibPixel + 1];
            image[y][x].B = data[ibPixel + 0];
            image_new[y][x].R = 0;
            image_new[y][x].G = 0;
            image_new[y][x].B = 0;
            ibPixel = ibPixel + 3;
        }
        ibRow = ibRow + liczba_pixeli_w_linii;
    }
    //
    printf("Sobel Algorytm Start...\n");
    for (unsigned int lin = 0; lin < bih.biHeight - 1; ++lin)
    {
        for (unsigned int col = 0; col < bih.biWidth - 1; ++col)
        {
            if (lin >= 1 && lin < bih.biHeight - 1 && col >= 1 && col < bih.biWidth - 1)
            {
                int ValBlueS1 = 0, ValBlueS2 = 0, ValBlueS3 = 0, ValBlueS4 = 0, ValBlueS5 = 0, ValBlueS6 = 0, ValBlueS7 = 0, ValBlueS8 = 0;
                int ValGreenS1 = 0, ValGreenS2 = 0, ValGreenS3 = 0, ValGreenS4 = 0, ValGreenS5 = 0, ValGreenS6 = 0, ValGreenS7 = 0, ValGreenS8 = 0;
                int ValRedS1 = 0, ValRedS2 = 0, ValRedS3 = 0, ValRedS4 = 0, ValRedS5 = 0, ValRedS6 = 0, ValRedS7 = 0, ValRedS8 = 0;
                for (unsigned int x = lin - 1; x <= lin + 1; x++) //3x3
                {
                    for (unsigned int y = col - 1; y <= col + 1; y++) //3x3
                    {
                        //Pixel Blue - Konwolucja dla składowej B
                        ValBlueS1 += (image[x][y].B * sobel_S1[x - lin + 1][y - col + 1]);
                        ValBlueS2 += (image[x][y].B * sobel_S2[x - lin + 1][y - col + 1]);
                        ValBlueS3 += (image[x][y].B * sobel_S3[x - lin + 1][y - col + 1]);
                        ValBlueS4 += (image[x][y].B * sobel_S4[x - lin + 1][y - col + 1]);
                        ValBlueS5 += (image[x][y].B * sobel_S5[x - lin + 1][y - col + 1]);
                        ValBlueS6 += (image[x][y].B * sobel_S6[x - lin + 1][y - col + 1]);
                        ValBlueS7 += (image[x][y].B * sobel_S7[x - lin + 1][y - col + 1]);
                        ValBlueS8 += (image[x][y].B * sobel_S8[x - lin + 1][y - col + 1]);
                        //Pixel Green - Konwolucja dla składowej G
                        ValGreenS1 += (image[x][y].G * sobel_S1[x - lin + 1][y - col + 1]);
                        ValGreenS2 += (image[x][y].G * sobel_S2[x - lin + 1][y - col + 1]);
                        ValGreenS3 += (image[x][y].G * sobel_S3[x - lin + 1][y - col + 1]);
                        ValGreenS4 += (image[x][y].G * sobel_S4[x - lin + 1][y - col + 1]);
                        ValGreenS5 += (image[x][y].G * sobel_S5[x - lin + 1][y - col + 1]);
                        ValGreenS6 += (image[x][y].G * sobel_S6[x - lin + 1][y - col + 1]);
                        ValGreenS7 += (image[x][y].G * sobel_S7[x - lin + 1][y - col + 1]);
                        ValGreenS8 += (image[x][y].G * sobel_S8[x - lin + 1][y - col + 1]);
                        //Pixel Red - Konwolucja dla składowej R
                        ValRedS1 += (image[x][y].R * sobel_S1[x - lin + 1][y - col + 1]);
                        ValRedS2 += (image[x][y].R * sobel_S2[x - lin + 1][y - col + 1]);
                        ValRedS3 += (image[x][y].R * sobel_S3[x - lin + 1][y - col + 1]);
                        ValRedS4 += (image[x][y].R * sobel_S4[x - lin + 1][y - col + 1]);
                        ValRedS5 += (image[x][y].R * sobel_S5[x - lin + 1][y - col + 1]);
                        ValRedS6 += (image[x][y].R * sobel_S6[x - lin + 1][y - col + 1]);
                        ValRedS7 += (image[x][y].R * sobel_S7[x - lin + 1][y - col + 1]);
                        ValRedS8 += (image[x][y].R * sobel_S8[x - lin + 1][y - col + 1]);
                    }
                }
                int Blue = (int)(abs(ValBlueS1) + abs(ValBlueS2) + abs(ValBlueS3) + abs(ValBlueS4) + abs(ValBlueS5) + abs(ValBlueS6) + abs(ValBlueS7) + abs(ValBlueS8)) / 8;
                int Green = (int)(abs(ValGreenS1) + abs(ValGreenS2) + abs(ValGreenS3) + abs(ValGreenS4) + abs(ValGreenS5) + abs(ValGreenS6) + abs(ValGreenS7) + abs(ValGreenS8)) / 8;
                int Red = (int)(abs(ValRedS1) + abs(ValRedS2) + abs(ValRedS3) + abs(ValRedS4) + abs(ValRedS5) + abs(ValRedS6) + abs(ValRedS7) + abs(ValRedS8)) / 8;
                if (Blue > 255)
                {
                    image_new[lin][col].B = WHITE_COLOR;
                }
                else {
                    image_new[lin][col].B = Blue;
                }
                if (Green > 255)
                {
                    image_new[lin][col].G = WHITE_COLOR;
                }
                else {
                    image_new[lin][col].G = Green;
                }
                if (Red > 255)
                {
                    image_new[lin][col].R = WHITE_COLOR;
                }
                else {
                    image_new[lin][col].R = Red;
                }
            }
            else
            {
                //ustalony kolor jednopixelowej ramki
                image_new[lin][col].B = BLACK_COLOR;
                image_new[lin][col].G = BLACK_COLOR;
                image_new[lin][col].R = BLACK_COLOR;
            }
        }
    }
    printf("Sobel Algorytm Koniec...\n");
    ibRow = 0;
    ibPixel = 0;
    //przepisanie danych z tablicydynamiczej image_new[][] w ktorej znajduja sie przetworzone wartosci pixeli obrazu do
    //wektora wyjsciowego fileData
    for (unsigned int y = 0; y < bih.biHeight; y = y + 1) {
        ibPixel = ibRow;
        for (unsigned int x = 0; x < bih.biWidth; x = x + 1) {
            fileData[ibPixel + 0] = image_new[y][x].B;
            fileData[ibPixel + 1] = image_new[y][x].G;
            fileData[ibPixel + 2] = image_new[y][x].R;
            ibPixel = ibPixel + 3;
        }
        ibRow = ibRow + liczba_pixeli_w_linii;
    }
    //zwolnienie pamieci dla tablic dynamicznych
    for (unsigned int i = 0; i < bih.biHeight; i++)
    {
        delete[] image[i];
        delete[] image_new[i];
    }
    delete[] image;
    delete[] image_new;
    //
    return fileData;
}
/*
* Funkcja tworzy plik BMP. Odczytuje struktury naglowka pliku BITMAPFILEHEADER i naglowka
* obrazu BITMAPINFOHEADER i zapisuje je w pliku dodajac vektor z danymi pixeli
* uzupelnionych wartosciami 0 w ilości zaleznej od paddingSize.
*/
void SaveBMPFile(ostream& os, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih, vector<char> data)
{
    uint32_t ilosc_bajtow_w_lini_obrazu{ 0 };
    //szerokosc w pixelach * 3(3 bajty na pixel)
    ilosc_bajtow_w_lini_obrazu = bih.biWidth * 3;
    uint32_t nowa_ilosc_bajtow_w_lini = ilosc_bajtow_w_lini_obrazu;
    while (nowa_ilosc_bajtow_w_lini % 4 != 0) {
        nowa_ilosc_bajtow_w_lini++;
    }
    //tworzenie wektora wyrowanania liczby bajtow w lini - uzupelnienie 0 w zaleznosci od wyniku
    //(nowa_ilosc_bajtow_w_lini - ilosc_bajtow_w_lini_obrazu)
    vector<uint8_t> wyrownanie_wiersza(nowa_ilosc_bajtow_w_lini - ilosc_bajtow_w_lini_obrazu);
    os.write(reinterpret_cast<char*>(&bfh), sizeof(bfh));
    os.write(reinterpret_cast<char*>(&bih), sizeof(bih));
    for (unsigned int y = 0; y < bih.biHeight; ++y) {
        os.write((const char*)(data.data() + (size_t)ilosc_bajtow_w_lini_obrazu * y), ilosc_bajtow_w_lini_obrazu);
        os.write((const char*)wyrownanie_wiersza.data(), wyrownanie_wiersza.size());
    }
}
/*
* Funkcja odczytuje dane pixeli obrazu z pliku obrazu BMP filename i zwraca je do
* wektora wyjsciowego. Funkcja pomija dodatkowe bajty wynikające z paddingu
*/
vector<char> readBMPPixelData(string filename, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih)
{
    // otwarcie pliku do odczytu
    ifstream inp(filename, ios::binary);
    // skok do danych pixeli
    inp.seekg(bfh.bfOffBits, inp.beg);
    //przygotowanie wektora do zapisu
    vector<char> fileData;
    //zmiana rozmiaru wektora na wszystkie dane pixeli
    fileData.resize(bih.biSizeImage);
    //zapis danych obrazu do wektora fileData ze strumienia inp
    if (bih.biWidth % 4 == 0) {
        //jezeli szerokosc obrazu bih.biWidth jest wielokrotnoscia liczby 4
        //to nie ma paddingu i odczyt danych obrazu jest ciagly
        inp.read((char*)fileData.data(), fileData.size());
    }
    else
    {
        //jezeli szerokosc obrazu bih.biWidth nie jest wielokrotnoscia liczby 4
        //to trzeba pominac bajty paddingu i nie wpisywac ich do wektora fileData
        //jednak trzeba je odczytac aby pozycja odczytu uwzgledniała padding aby
        //padding nie znalazl razem z danymi obrazu
        uint32_t ilosc_bajtow_w_lini_obrazu{ 0 };
        //szerokosc w pixelach * 3(3 bajty na pixel)
        ilosc_bajtow_w_lini_obrazu = bih.biWidth * bih.biBitCount / 8;
        uint32_t nowa_ilosc_bajtow_w_lini = ilosc_bajtow_w_lini_obrazu;
        while (nowa_ilosc_bajtow_w_lini % 4 != 0) {
            nowa_ilosc_bajtow_w_lini++;
        }
        //tworzenie wektora wyrowanania liczby bajtow w lini - uzupelnienie 0 w zaleznosci od wyniku
        //(nowa_ilosc_bajtow_w_lini - ilosc_bajtow_w_lini_obrazu)
        vector<uint8_t> wyrownanie_wiersza(nowa_ilosc_bajtow_w_lini - ilosc_bajtow_w_lini_obrazu);
        for (unsigned int y = 0; y < bih.biHeight; ++y) {
            inp.read((char*)(fileData.data() + (size_t)ilosc_bajtow_w_lini_obrazu * y), ilosc_bajtow_w_lini_obrazu);
            inp.read((char*)wyrownanie_wiersza.data(), wyrownanie_wiersza.size());
        }
    }
    return fileData;
}
/*
* Funkcja czyta dane obrazu z pliku "filename" bez paddingu i zwraca je w postaci wektora elementów char.
* Dane obrazu sa dzielone na czesci wzgledem podanych parametrow h1 i h2. Parametry h1 i h2 okreslaja poczatek i koniec
* odczytu danych z obrazu.
* Funkcja wykorzystywana do czytania pliku obrazu po kawałku w celu przetwarzabia obrazow o duzej wielkosci.
*   biHeight    *   *   *   *   *
*       *       *   *   *   *   *
*       *       *   *   *   *   *
*       h2      -   -   -   -   -   \
*       *       -   -   -   -   -    \ obszar odczytywanych danych
*       *       -   -   -   -   -    /
*       h1      -   -   -   -   -   /
*       *       *   *   *   *   *
*       1       *   *   *   *   *
*       0       1   2   *   *   biWidth
*/
vector<char> readBMPPixelDataPosision(string filename, BITMAPFILEHEADER& bfh, BITMAPINFOHEADER& bih, int h1, int h2)
{
    // open the file:
    ifstream inp(filename, ios::binary);
    vector<char> fileData;
    bih.biSizeImage = ((h2 - h1 + 1) * bih.biWidth * 3);
    fileData.resize(bih.biSizeImage);
    uint32_t ilosc_bajtow_w_lini_obrazu = bih.biWidth * bih.biBitCount / 8;    
    uint32_t nowa_ilosc_bajtow_w_lini = ilosc_bajtow_w_lini_obrazu;
    while (nowa_ilosc_bajtow_w_lini % 4 != 0) {
        nowa_ilosc_bajtow_w_lini++;
    }
    //tworzenie wektora wyrowanania liczby bajtow w lini - uzupelnienie 0 w zaleznosci od wyniku
    //(nowa_ilosc_bajtow_w_lini - ilosc_bajtow_w_lini_obrazu)
    vector<uint8_t> wyrownanie_wiersza(nowa_ilosc_bajtow_w_lini - ilosc_bajtow_w_lini_obrazu);
    if (bih.biWidth % 4 == 0) {
        inp.seekg((double)bfh.bfOffBits + ((double)h1 * (double)bih.biWidth * 3), inp.beg);
        int bufor_size = bih.biWidth * (h2 - h1 + 1) * 3;
        inp.read((char*)fileData.data(), bih.biSizeImage);
    }
    else
    {
        inp.seekg((double)bfh.bfOffBits + ((double)h1 * ((double)bih.biWidth * 3 + wyrownanie_wiersza.size())), inp.beg);
        for (unsigned int y = 0; y <= (size_t)h2 - (size_t)h1; ++y) {
            inp.read((char*)(fileData.data() + (size_t)ilosc_bajtow_w_lini_obrazu * (size_t)y), ilosc_bajtow_w_lini_obrazu);
            inp.read((char*)wyrownanie_wiersza.data(), wyrownanie_wiersza.size());
        }
    }
    inp.close();
    return fileData;
}
/*Funkcja odczytuje naglowek pliku BMP ze strumienia ifstream, zapisuje informacje w polach struktury BITMAPFILEHEADER i zwraca bieżącą
pozycję kursora w strumieniu*/
int odczytajBFH(ifstream& ifs, BITMAPFILEHEADER& bfh)
{
    ifs.read(reinterpret_cast<char*>(&bfh.bfType), sizeof(bfh.bfType));
    ifs.read(reinterpret_cast<char*>(&bfh.bfSize), sizeof(bfh.bfSize));
    ifs.read(reinterpret_cast<char*>(&bfh.bfReserved1), sizeof(bfh.bfReserved1));
    ifs.read(reinterpret_cast<char*>(&bfh.bfReserved2), sizeof(bfh.bfReserved2));
    ifs.read(reinterpret_cast<char*>(&bfh.bfOffBits), sizeof(bfh.bfOffBits));
    return (int)ifs.tellg();
}
/*Funkcja wyświetla na ekranie dane struktury BITMAPFILEHEADER*/
void PrintBFH(BITMAPFILEHEADER& bfh)
{
    printf("\n INFORMACJE O BITMAPIE\n\n");
    printf(" Sygnatura pliku:...............................%x", bfh.bfType);
    printf("\n Rozmiar pliku:................................%d bajtow", bfh.bfSize);
    printf("\n Pole zarezerwowane1:..........................%d", bfh.bfReserved1);
    printf("\n Pole zarezerwowane2:..........................%d", bfh.bfReserved2);
    printf("\n Pozycja danych obrazowych:....................%d", bfh.bfOffBits);
    printf("\n");
}
/*Funkcja odczytuje naglowek obrazu BMP ze strumienia ifstream, zapisuje informacje w polach struktury BITMAPFILEHEADER i zwraca bieżącą
pozycję kursora w strumieniu */
int odczytajBIH(ifstream& ifs, BITMAPINFOHEADER& bih)
{
    ifs.read(reinterpret_cast<char*>(&bih.biSize), sizeof(bih.biSize));
    ifs.read(reinterpret_cast<char*>(&bih.biWidth), sizeof(bih.biWidth));
    ifs.read(reinterpret_cast<char*>(&bih.biHeight), sizeof(bih.biHeight));
    ifs.read(reinterpret_cast<char*>(&bih.biPlanes), sizeof(bih.biPlanes));
    ifs.read(reinterpret_cast<char*>(&bih.biBitCount), sizeof(bih.biBitCount));
    ifs.read(reinterpret_cast<char*>(&bih.biCompression), sizeof(bih.biCompression));
    ifs.read(reinterpret_cast<char*>(&bih.biSizeImage), sizeof(bih.biSizeImage));
    ifs.read(reinterpret_cast<char*>(&bih.biXpelsPerMeter), sizeof(bih.biXpelsPerMeter));
    ifs.read(reinterpret_cast<char*>(&bih.biYpelsPerMeter), sizeof(bih.biYpelsPerMeter));
    ifs.read(reinterpret_cast<char*>(&bih.biCrlUses), sizeof(bih.biCrlUses));
    ifs.read(reinterpret_cast<char*>(&bih.biCrlImportant), sizeof(bih.biCrlImportant));
    return (int)ifs.tellg();
}
/*Funkcja wyświetla na ekranie dane struktury BITMAPINFOHEADER*/
void PrintBIH(BITMAPINFOHEADER& bih)
{
    printf("\n Wielkosc naglowka informacyjnego:.............%d", bih.biSize);
    printf("\n Szerokosc:....................................%d pikseli", bih.biWidth);
    printf("\n Wysokosc:.....................................%d pikseli", bih.biHeight);
    printf("\n Liczba platow (zwykle 0): ....................%d", bih.biPlanes);
    printf("\n Liczba bitow na piksel: ......................%d (1, 4, 8, or 24)", bih.biBitCount);
    printf("\n Kompresja: ...................................%d (0=none, 1=RLE-8, 2=RLE-4)", bih.biCompression);
    printf("\n Rozmiar samego rysunku: ......................%d", bih.biSizeImage);
    printf("\n Rozdzielczosc pozioma: .......................%d", bih.biXpelsPerMeter);
    printf("\n Rozdzielczosc pionowa: .......................%d", bih.biYpelsPerMeter);
    printf("\n Liczba kolorow w palecie: ....................%d", bih.biCrlUses);
    printf("\n Wazne kolory w palecie: ......................%d", bih.biCrlImportant);
    printf("\n");
}
int sprawdzczyint(string naturalna) 
{
    bool liczba = true;
    for (int i = 0; i < naturalna.length(); i++)
    {
        if (!isdigit(naturalna[i]))
            liczba = false;
    }
    if (liczba)
        return 1;
    else
        return 0;
}
void help1() {
    cout << "************************************************************************************************" << endl;
    cout << "*                     \"Detekcja krawedzi operatorem Sobela\"                                    *" << endl;
    cout << "************************************************************************************************" << endl;
    cout << "*  Program umozliwia detekcje krawedzi w 24-bitowej bitmapie z wykorzystaniem                  *" << endl;
    cout << "*  operatora Sobela.                                                                           *" << endl;
    cout << "*                                                                                              *" << endl;
    cout << "*   Mozliwe do wyboru opcje:                                                                   *" << endl;
    cout << "*   ------------------------                                                                   *" << endl;
    cout << "* --> Odczytanie wszystkich parametrow pliku z naglowka obrazu BMP i wyświetlenie              *" << endl;
    cout << "*   ich ma ekranie                                                                             *" << endl;
    cout << "*                                                                                              *" << endl;
    cout << "* --> Detekcja krawedzi z wykorzystaniem operatora Sobela, dla odczytanego pliku               *" << endl;
    cout << "*   ,oraz zapis wyniku w pliku BMP(nazwe podaje uzytkownik).                                   *" << endl;
    cout << "*                                                                                              *" << endl;
    cout << "* --> Wczytywanie kolejnych fragmentow bitmapy o okreslonej wysokosci, oraz przeprowadzenie    *" << endl;
    cout << "*   na nich detekcji krawedzi operatorem Sobela. Wynikowy plik to \"sklejone\" fragmenty         *" << endl;
    cout << "*   przetworzonej bitmapy. Wynikowy obraz zawiera ramke.                                       *" << endl;
    cout << "*                                                                                              *" << endl;
    cout << "* --> Obsluga filtrow o dowolnej liczbie i wielkosci maski. Program umozliwia wybor rozmiaru   *" << endl;
    cout << "*   maski (3x3; 5x5; 7x7; 9x9), oraz ich ilosci 1-8.                                           *" << endl;
    cout << "*   Dodatkowo same maski przechowywane sa w plikach(nazwa pliku to np. Sobel_S1_7x7 - pierwsza *" << endl;
    cout << "*   maska o wymiarach 7x7                                                                      *" << endl;
    cout << "************************************************************************************************" << endl;
}
void help2() {
    cout << "**********************************************************************************************" << endl;
    cout << "*                             Format pliku maski/filtru                                      *" << endl;
    cout << "**********************************************************************************************" << endl;
    cout << "* Szerokosc                                                                                  *" << endl;
    cout << "* Wysokosc                                                                                   *" << endl;
    cout << "* 1 element 1 rzedu [spacja] ........(n-ty element 1 rzedu)                                  *" << endl;
    cout << "* .                                                                                          *" << endl;
    cout << "* .                                                                                          *" << endl;
    cout << "* .                                                                                          *" << endl;
    cout << "* 1 element n-tego rzedu .......(n-ty element n-tego rzedu)                                  *" << endl;
    cout << "*                                                                                            *" << endl;
    cout << "* // Przyklad dla pierwszej maski 5x5 //                                                     *" << endl;
    cout << "* //lokalizacjs: /5x5/Sobel_S1_5x5.txt//                                                     *" << endl;
    cout << "* 5                                                                                          *" << endl;
    cout << "* 5                                                                                          *" << endl;
    cout << "* -2  -1  0 1 2                                                                              *" << endl;
    cout << "* -3  -2  0 2 3                                                                              *" << endl;
    cout << "* -4  -3  0 3 4                                                                              *" << endl;
    cout << "* -3  -2  0 2 3                                                                              *" << endl;
    cout << "* -2  -1  0 1 2                                                                              *" << endl;
    cout << "**********************************************************************************************" << endl;
}
int pomoc()
{
    int i = 0;
    string ii;
    for (;;)
    {
        system("cls");
        cout <<"*****************************************" << endl;
        cout <<"* Opcje:                                *" << endl;
        cout <<"* 1 - Opis programu                     *" << endl;
        cout <<"* 2 - Budowa pliku maski                *"  << endl;
        cout <<"* 3 - Powrot do menu glownego           *" << endl;
        cout <<"*****************************************" << endl;
        cout <<"Wybierz opcje:" << endl;
        cin >> ii;
        while (!sprawdzczyint(ii))
        {
            cout << "Zly wybor, wprowadz jeszcze raz (Nie wprowadzono liczby)" << endl;
            cin >> ii;
        }
        switch (stoi(ii))
        {
        case 1:
            system("cls");
            help1();
            system("pause");
            break;
        case 2:
            system("cls");
            help2();
            system("pause");
            break;
        case 3:
            return(1);
        default:
            cout << "Mozliwe do wyboru opcje to 1 i 2";
            system("pause");
            break;
        }
    }
}
/*Program glowny*/
int main()
{
    //deklaracja struktur do przechowywania danych pliku BMP
    BITMAPFILEHEADER bmp_file_header;
    BITMAPINFOHEADER bmp_info_header;
    menu(bmp_file_header, bmp_info_header);
    return 1;
}
