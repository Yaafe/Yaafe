#!/bin/env python
# coding: UTF-8

import os
import sys
from yaafelib import FeaturePlan, Engine, AudioFileProcessor 
import numpy as np

SLICE_WINSIZE = 4096
SLICE_STEPSIZE = 4096
SAMPLE_RATE = 44100


TIME_START = -10
TIME_LIMIT = 0


def main(argv):
    if len(argv) != 2:
        print 'usage: python %s foo.mp3' % (argv[0])
        return
    fp = FeaturePlan(sample_rate=SAMPLE_RATE, resample=True, time_start=TIME_START, time_limit=TIME_LIMIT)
    fp.addFeature("frames: Frames blockSize={0} stepSize={1}".format(
        SLICE_WINSIZE, SLICE_STEPSIZE))
    if 'YAAFE_PATH' in os.environ:
        fp.addFeature(
            "beat_hist: BeatHistogramSummary ACPNbPeaks=3  BHSBeatFrameSize=128  BHSBeatFrameStep=64  "
            "BHSHistogramFrameSize=40  BHSHistogramFrameStep=40  FFTLength=0  FFTWindow=Hanning  "
            "HInf=40  HNbBins=80  HSup=200  NMANbFrames=5000  blockSize=1024  stepSize=512")
    df = fp.getDataFlow()
    engine = Engine()
    engine.load(df)
    afp = AudioFileProcessor()

    song_path = argv[1]
    assert os.path.exists(song_path)
    afp.processFile(engine, song_path)
    frames = engine.readOutput('frames')
    frames = np.concatenate(frames)
    print 'time start: %ss' % TIME_START
    print 'time limit: %ss' % TIME_LIMIT
    print 'duration:', 1. * frames.size / SAMPLE_RATE

    if 'YAAFE_PATH' in os.environ:
        beat_hist = engine.readOutput('beat_hist')
        print 'beat_hist: %s' % beat_hist


    # from pylab import plot, show
    # plot(frames)
    # show()


if __name__ == '__main__':
    main(sys.argv)
