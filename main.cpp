#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <matplot/matplot.h>
#include "AudioFile/AudioFile.h"
#include <math.h>

void sinus(double width=6.28, double frequency=1.0, int precision=100) {
    using namespace matplot;
    std::vector<double> x = linspace(0, width, (int)(precision * frequency));
    std::vector<double> y = transform(x, [&frequency](auto x) { return sin(frequency * x); });
    plot(x, y);
    ylim({-2,2});
    show();
}

void cosinus(double width=6.28, double frequency=1.0, int precision=100) {
    using namespace matplot;
    std::vector<double> x = linspace(0, width, (int)(precision * frequency));
    std::vector<double> y = transform(x, [&frequency](auto x) { return cos(frequency * x); });
    plot(x, y);
    ylim({-2,2});
    show();
}

void rectangularSignal(double frequency){
    using namespace matplot;
    std::vector<double> x = linspace(0, 10, (int)(10*2*frequency+1));
    std::vector <double> y;
    for(int i=0;i!=(int)(10*2*frequency+1);i++){
        y.push_back(i%2);
    }
    stairs(x,y);
    ylim({-0.5,1.5});
    grid(1);
    show();
}

void sawTooth(double frequency){
    using namespace matplot;
    fplot([&frequency](double x) {return 2.0 * (x * frequency - std::floor(0.5 + x * frequency)); });
    ylim({ -1, 1 });
    show();
}

bool showWave(std::string &path){
    AudioFile <double> audio;
    if(!audio.load(path))
        return 0;
    std::vector<double>x = matplot::linspace(0, audio.getLengthInSeconds(), audio.getSampleRate());
    matplot::title(path);
    matplot::xlabel("Time [s]");
    matplot::grid(1);
    matplot::plot(x,audio.samples[0]);
    matplot::ylim({ -2, 2 });
    matplot::show();
    return 1;
}

bool oneDConvolution(std::string &inPath, std::vector <double> &kernel){
    AudioFile <double> audio;
    if(!audio.load(inPath))
        return 0;
    int n = audio.samples[0].size();
    int m = kernel.size();
    std::vector <double> output(n, 0.0);

    for(int i=0;i!=n;i++){
        for(int j = 0; j!=m;j++){
            if(i-j>=0){
                output[i]+=audio.samples[0][i-j]*kernel[j];
            }
        }
    }
    std::vector<double>x = matplot::linspace(0, audio.getLengthInSeconds(), audio.getSampleRate());
    matplot::plot(x,output);
    matplot::title("Filtered: "+inPath);
    matplot::xlabel("Time [s]");
    matplot::ylim({ -2, 2 });
    matplot::show();
    int size=audio.getNumSamplesPerChannel();
    for(int i=0;i!=size;i++){
        audio.samples[0][i]=output[i];
    }
    audio.save("test.wav");
    return 1;
}

bool crossCorrelation(std::string &path1, std::string &path2){
    AudioFile <double> audio1, audio2;
    if(!audio1.load(path1)||!audio2.load(path2))
        return 0;
    int size, size1=audio1.getNumSamplesPerChannel(), size2=audio2.getNumSamplesPerChannel();
    if(size1>size2){
        size=size2;
    }else{
        size=size1;
    }
    std::vector<double> crossCorr(size, 0.0);
    int k;
    for (int i = 0; i != size; i++) {
        for (int j = 0; j != size; j++) {
            k = (j + i) % size;
            crossCorr[i] += audio1.samples[0][j] * audio2.samples[0][k];
        }
    }
    std::vector<double>x = matplot::linspace(0, audio1.getLengthInSeconds(), audio1.getSampleRate());
    matplot::plot(x,crossCorr);
    matplot::title("Cross correlation of: "+path1+" and "+path2);
    matplot::xlabel("Time [s]");
    matplot::ylim({ -2, 2 });
    matplot::show();
    return 1;
}

PYBIND11_MODULE(signals, handle){
    handle.doc() = "Biblioteka sluzy do obslugi sygnalow (wczytywanie z plikow .wav lub .aiff; generowanie sygnalow roznego rodzaju)";
    handle.def("sin", &sinus, "Funkcja rysuje sygnal sinus w zaleznosci od ewentualnych parametrow", pybind11::arg("width") = 6.28, pybind11::arg("frequency") = 1.0, pybind11::arg("precision") = 100);
    handle.def("cos", &cosinus, "Funkcja rysuje sygnal funkcji cosinus w zaleznosci od ewentualnych parametrow", pybind11::arg("width") = 6.28, pybind11::arg("frequency") = 1.0, pybind11::arg("precision") = 100);
    handle.def("rectangularSignal", &rectangularSignal, "Funkcja wyswietla sygnal prostokatny w zaleznosci od czestotliwosci", pybind11::arg("frequency") = 1.0);
    handle.def("sawTooth", &sawTooth, "Funkcja wyswietla sygnal piloksztaltny w zaleznosci od czestotliwosci", pybind11::arg("frequency")=1.0);
    handle.def("showWave", &showWave, "Funkcja wyswietla sygnal wczystany z pliku .wav lub .aiff", pybind11::arg("path"));
    handle.def("oneDConvolution", &oneDConvolution, "Funckja filtruje sygnal wczytany z pliku .wav lub .aiff w zaleznosci od wartosci z listy kernel. Funkcja generuje plik test.wav do sprawdzenia poprawnosci filtracji", pybind11::arg("inPath"), pybind11::arg("kernel"));
    handle.def("crossCorrelation", &crossCorrelation, "Funckja liczy korelacje miedzy dwoma sygnmalami wczytanymi z plikow .wav lub .aiff", pybind11::arg("path1"), pybind11::arg("path2"));
}

