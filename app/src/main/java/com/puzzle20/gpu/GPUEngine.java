package com.puzzle20.gpu;

public class GPUEngine {
    static { System.loadLibrary("puzzle20_gpu"); }
    public native boolean initGPU();
    public native void releaseGPU();
}
