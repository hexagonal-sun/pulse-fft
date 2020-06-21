#include <pulse/simple.h>
#include <pulse/error.h>
#include <iostream>
#include <array>
#include <gsl/gsl>
#include <complex>
#include <fftw3.h>
#include <cmath>

template<std::size_t bufferSz>
int __main()
{
    pa_sample_spec ss = {};
    int error;

    ss.channels = 2;
    ss.rate = 44100;
    ss.format = PA_SAMPLE_FLOAT32;

    auto server = pa_simple_new(NULL, "pulse-fft", PA_STREAM_RECORD,
                                NULL, "playback", &ss, NULL,
                                NULL, &error);

    if (!server)
    {
        std::cerr << "Error: could not connect to pulse audio server: " <<
            pa_strerror(error);

        return 1;
    }

    auto closeStream = gsl::finally([&server] {
        pa_simple_free(server);
    });

    std::array<std::complex<double>, (bufferSz >> 1)> fftArray;

    auto plan = fftw_plan_dft_1d(bufferSz >> 1, reinterpret_cast<fftw_complex *>(&fftArray[0]),
                                 reinterpret_cast<fftw_complex *>(&fftArray[0]), FFTW_FORWARD, 0);
    auto deletePlan = gsl::finally([&plan] {
        fftw_destroy_plan(plan);
    });

    while (1) {
        std::array<float, bufferSz> pcmBuf;

        pa_simple_read(server, &pcmBuf[0], sizeof(pcmBuf), &error);

        for (size_t channel = 0; channel < 2; channel++)
        {
            for (size_t fftIdx = 0, pcmIdx = channel;
                 fftIdx < (bufferSz >> 1);
                 fftIdx++, pcmIdx += 2)
                fftArray[fftIdx] = std::complex<double>(pcmBuf[pcmIdx], 0);

            fftw_execute(plan);

            // Calculate RGB values from the FFT.
            unsigned int red = 0, green = 0, blue = 0;
            size_t fftIdx = 0;

            for (auto const &fftVal : fftArray) {
                double mag = std::abs(fftVal);

                if(fftIdx <= bufferSz >> 8)
                    red += mag;
                else if(fftIdx <= bufferSz >> 5)
                    green += mag;
                else {
                    blue += mag;
                }
                fftIdx++;
            }

            red = std::floor(((double)red / 200) * 255);
            green = std::floor(((double)green / 400) * 255);
            blue = std::floor(((double)blue / 800) * 255);

            if (red > 255)
                red = 255;

            if (green > 255)
                green = 255;

            if (blue > 255)
                blue = 255;

            std::cout << red << "," << green << "," << blue << " ";
        }
        std::cout << "\n" << std::flush;
    }
}

int main()
{
    return __main<1024>();
}
