static OSStatus audioDeviceIOProc(

    AudioDeviceID inDevice,

    const AudioTimeStamp* inNow,

    const AudioBufferList* inInputData,

    const AudioTimeStamp* inInputTime,

    AudioBufferList* outOutputData,

    const AudioTimeStamp* inOutputTime,

    void* hwptr)

{

    UInt32 frame, frameCount;

    float *out = outOutputData->mBuffers[0].mData;

    HWVoiceOut *hw = hwptr;

    coreaudioVoiceOut *core = (coreaudioVoiceOut *) hwptr;

    int rpos, live;

    st_sample_t *src;

#ifndef FLOAT_MIXENG

#ifdef RECIPROCAL

    const float scale = 1.f / UINT_MAX;

#else

    const float scale = UINT_MAX;

#endif

#endif



    if (coreaudio_lock (core, "audioDeviceIOProc")) {

        inInputTime = 0;

        return 0;

    }



    frameCount = core->audioDevicePropertyBufferFrameSize;

    live = core->live;



    

    if (live < frameCount) {

        inInputTime = 0;

        coreaudio_unlock (core, "audioDeviceIOProc(empty)");

        return 0;

    }



    rpos = core->rpos;

    src = hw->mix_buf + rpos;



    

    for (frame = 0; frame < frameCount; frame++) {

#ifdef FLOAT_MIXENG

        *out++ = src[frame].l; 

        *out++ = src[frame].r; 

#else

#ifdef RECIPROCAL

        *out++ = src[frame].l * scale; 

        *out++ = src[frame].r * scale; 

#else

        *out++ = src[frame].l / scale; 

        *out++ = src[frame].r / scale; 

#endif

#endif

    }



    rpos = (rpos + frameCount) % hw->samples;

    core->decr += frameCount;

    core->rpos = rpos;



    coreaudio_unlock (core, "audioDeviceIOProc");

    return 0;

}