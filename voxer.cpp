#include "voxer.h"
using namespace std::chrono;
#include "r8b/r8bsrc.h"

float remap(float OldValue, float OldMin, float OldMax, float NewMin, float NewMax ){

    float NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;

    return NewValue;

}

std::vector<float> Resample(const std::vector<float>& InAudata,uint32_t SrcSampleRate,uint32_t OutSampleRate)
{
    if (SrcSampleRate == OutSampleRate)
        return InAudata;

    // Define the resampler

    int32_t SampleCount = (int32_t)InAudata.size();


    // 2.5 is a good middle-ground number for this parameter whose name I just forgot
    CR8BResampler Resampler = r8b_create((double)SrcSampleRate,(double)OutSampleRate,SampleCount,2.5,ER8BResamplerRes::r8brr24);

    double* OutBuff = nullptr;

    std::vector<double> DBuff;
    DBuff.resize(InAudata.size());

    // Cast input buffer to double
    for (size_t i = 0; i < InAudata.size();i++)
        DBuff[i] = (double)InAudata[i];

    int32_t NumSamples = r8b_process(Resampler,DBuff.data(),SampleCount,OutBuff);

    // Create output buffer
    std::vector<float> OutAud;
    OutAud.resize((size_t)NumSamples);


    // Re-cast to float
    for (size_t i = 0; i < (size_t)NumSamples;i++)
        OutAud[i] = (float)OutBuff[i];


    // Cleanup
    r8b_clear(Resampler);
    r8b_delete(Resampler);


    return OutAud;





}

std::vector<float> DoDenoise(const std::vector<float>& InAudata,DenoiseState* DenState)
{
 //   if (!DenState)
   //     return InAudata;

    std::vector<float> NewAudata(InAudata.size());
    float buf[RNNoiseFrameSize];

    // Find the min and max vals in the vector
    float MinVal = *std::min_element(InAudata.begin(), InAudata.end());
    float MaxVal = *std::max_element(InAudata.begin(), InAudata.end());

    for (size_t f = 0; f < InAudata.size();f += RNNoiseFrameSize)
    {
        //RNNoise expects a float in range [-32768.f,32768.f]
        for (size_t y = 0; y < RNNoiseFrameSize;y++)
            buf[y] = remap(InAudata[f + y],MinVal,MaxVal,-32768.f,32768.f);


        rnnoise_process_frame(DenState,buf,buf);

        for (size_t x = 0; x < RNNoiseFrameSize;x++)
        {
            size_t TotalIndex = f + x;
            if (TotalIndex > NewAudata.size())
                break;

            NewAudata[TotalIndex] = remap(buf[x],-32768.f,32768.f,MinVal,MaxVal);

        }



    }

    // Subtract some samples because sometimes there can be a pop sound at the end.
    NewAudata.assign(NewAudata.begin(),NewAudata.end() - 300);

    return NewAudata;
}

void Voxer::run()
{

    pAttItem->setBackgroundColor(InProcessColor);

    high_resolution_clock::time_point Start = high_resolution_clock::now();
    std::vector<float> Audat = pAttVoice->Vocalize(Prompt.toStdString(),Speed,SpeakerID,Energy,F0,EmotionID);
    high_resolution_clock::time_point End = high_resolution_clock::now();


    // Resample the audio to 48KHz
    std::vector<float> AudRes = Resample(Audat,SampleRate,CommonSampleRate);


    DenoiseState* Denoiser = nullptr;
    if (Denoise)
    {
        // Every thread creates its own denoiser.
        // This is because a generic passed denoiser created from the main window
        // worked well for the first generation but later shat itself (heavy artifacts then just silence)

        Denoiser = rnnoise_create(nullptr);
        // Denoise. Function will return same vec if there is no denoiser
        AudRes = DoDenoise(AudRes,Denoiser);




    }


    // Apply Amplification
    for (float& f : AudRes)
        f *= Amplify;


    pAttItem->setBackgroundColor(DoneColor);
    emit Done(AudRes,duration_cast<duration<double>>(End - Start),CurrentID);

    // rnnoise_destroy throws some exception we can't do anything about
    if (Denoise)
    {
        try {
            rnnoise_destroy(Denoiser);

        } catch (...) {

        }

    }

}

Voxer::Voxer()
{

}
