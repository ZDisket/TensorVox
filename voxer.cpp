#include "voxer.h"
using namespace std::chrono;
#include "r8b/r8bsrc.h"

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

void Voxer::run()
{

    pAttItem->setBackgroundColor(InProcessColor);

    high_resolution_clock::time_point Start = high_resolution_clock::now();
    std::vector<float> Audat = pAttVoice->Vocalize(Prompt.toStdString(),Speed,SpeakerID,Energy,F0);
    high_resolution_clock::time_point End = high_resolution_clock::now();


    // Resample the audio to 48KHz
    std::vector<float> AudRes = Resample(Audat,SampleRate,CommonSampleRate);

    pAttItem->setBackgroundColor(DoneColor);
    emit Done(AudRes,duration_cast<duration<double>>(End - Start));

}

Voxer::Voxer()
{

}
