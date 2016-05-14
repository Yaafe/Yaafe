# -*- coding: utf-8 -*-
# Yaafe
#
# Copyright (c) 2009-2010 Institut Telecom - Telecom Paristech
# Telecom ParisTech / dept. TSI
#
# Author : Benoit Mathieu
#
# This file is part of Yaafe.
#
# Yaafe is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Yaafe is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

'''
Yaafe core audio features.
'''

from __future__ import absolute_import, print_function

from yaafelib.audiofeature import (AudioFeature, check_dataflow_params,
                                   dataflow_safe_append)
from yaafelib.dataflow import DataFlow
import math
import decimal


class Frames(AudioFeature):

    '''
    Segment input signal into frames.

    First frame has zeros on left half so that it is centered on time 0s,
    then consecutive frames are equally spaced.  Consequently,
    frame *i* (starting from 0) is centered on sample *i* * *stepSize*.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('FrameTokenizer', {}), ]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('FrameTokenizer', params)
        return df


class ZCR(AudioFeature):

    '''
    Compute zero-crossing rate in frames. see [SS1997]_.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}), ]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df, 'ZCR', {})
        return df


class TemporalShapeStatistics(AudioFeature):

    '''
    Compute :ref:`shape statistics <shapestatistics>` of signal frames.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}), ]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df, 'ShapeStatistics', {})
        return df


class Energy(AudioFeature):

    '''
    Compute energy as root mean square of an audio Frame.

    .. math:: en = \sqrt\\frac{\sum_{i=0}^{N-1}x(i)^2}{N}

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}), ]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df, 'RMS', {})
        return df


class MagnitudeSpectrum(AudioFeature):

    '''
    Compute frame's magnitude spectrum, using an analysis window (Hanning
    or Hamming), or not.

    .. math:: y = abs(fft(w*x))
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}),
              ('FFT', {'FFTLength': '0'})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        if (params['FFTLength'] == '0'):
            params['FFTLength'] = params['blockSize']
        dataflow_safe_append(df, 'FFT', params)
        dataflow_safe_append(df, 'Abs', {})
        return df


class SpectralVariation(AudioFeature):

    '''
    SpectralVariation is the normalized correlation of
    :class:`spectrum <MagnitudeSpectrum>` between consecutive frames.
    (see [GP2004]_)

    .. math:: S_{var} = 1 - \\frac{\sum_{k}a_{k}(t-1)a_{k}(t)}
                                  {\sqrt{\sum_{k}a_{k}(t-1)^2}
                                  \sqrt{\sum_{k}a_{k}(t)^2}}

    .. [GP2004] Geoffroy Peeters, *A large set of audio features for sound
                description (similarity and classification) in
                the CUIDADO project*, 2004.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Variation', params)
        return df


class SpectralSlope(AudioFeature):

    '''
    SpectralSlope is computed by linear regression of the spectral amplitude.
    (see [GP2004]_)

    .. math:: S_{slope} = \\frac{K\sum_{k}f_{k}a_{k}
                                 -\sum_{k}f_{k}\sum_{k}a_{k}}
                                {K\sum_{k}f_{k}^2-(\sum_{k}a_{k})^2}
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Slope', params)
        return df


class SpectralRolloff(AudioFeature):

    '''
    Spectral roll-off is the frequency so that 99% of the energy is contained
    below. see [SS1997]_.

    .. [SS1997] E.Scheirer, M.Slaney. *Construction and evaluation of a robust
                multifeature speech/music discriminator*. IEEE Internation
                Conference on Acoustics, Speech and Signal Processing,
                p.1331-1334, 1997.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Rolloff', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Sqr', {})
        dataflow_safe_append(df, 'Rolloff', params)
        return df


class SpectralShapeStatistics(AudioFeature):

    '''
    Compute shape statistics of :class:`MagnitudeSpectrum`, (see [GR2004]_).

    .. _shapestatistics:

    **Shape Statistics** are centroid, spread, skewness and kurtosis, defined
    as follow:

    .. math::

        \mu_{i} &= \\frac{\sum_{n=1}^{N}f_{k}^{i}*a_{k}}
                         {\sum_{n=1}^{N}a_{k}}\\\\
        centroid &= \mu_{1}\\\\
        spread &= \sqrt{\mu_{2}-\mu_{1}^{2}} \\\\
        skewness &= \\frac{2\mu_{1}^{3} - 3\mu_{1}\mu_{2} + \mu_{3}}
                          {spread^{3}} \\\\
        kurtosis &= \\frac{-3\mu_{1}^{4} + 6\mu_{1}\mu_{2} - 4\mu_{1}\mu_{3}
                           + \mu_{4}}{spread^{4}} - 3

    .. [GR2004] O.Gillet, G.Richard, *Automatic transcription of drum loops.*
                in IEEE International Conference on Acoustics, Speech and
                Signal Processing (ICASSP), Montreal, Canada, 2004.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('ShapeStatistics', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'ShapeStatistics', params)
        return df


class SpectralFlux(AudioFeature):

    '''
    Compute flux of :class:`spectrum <MagnitudeSpectrum>` between consecutives
    frames.

    .. math:: S_{flux} = \\frac{\sum_{k}(a_{k}(t) - a_{k}(t-1))^2}
                               {\sqrt{\sum_{k}a_{k}(t-1)^2}
                                \sqrt{\sum_{k}a_{k}(t)^2}}

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Flux', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(
            MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df, 'Flux', params)
        return df


class SpectralFlatness(AudioFeature):

    '''
    Compute global spectral flatness using the ratio between geometric and
    arithmetic mean.

    .. math:: S_{flatness} = \\frac{exp(\\frac{1}{N}\sum_{k}log(a_{k}))}
                                   {\\frac{1}{N}\sum_{k}a_{k}}
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Flatness', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Flatness', params)
        return df


class SpectralDecrease(AudioFeature):

    '''
    Compute spectral decrease accoding to [GP2004]_.

    .. math:: S_{decrease} = \\frac{1}{\sum_{k=2}^{K}a_{k}}
                              \sum_{k=2}^{K}\\frac{a_{k}-a_{1}}{k-1}
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Decrease', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Decrease', params)
        return df


class SpectralFlatnessPerBand(AudioFeature):

    '''
    Compute spectral flatness per log-spaced band of 1/4 octave, as proposed
    in MPEG7 standard.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Sqr', {}),
              ('SpectralFlatnessPerBand', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Sqr', {})
        dataflow_safe_append(df, 'SpectralFlatnessPerBand', params)
        return df


class SpectralCrestFactorPerBand(AudioFeature):

    '''
    Compute spectral crest factor per log-spaced band of 1/4 octave.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Sqr', {}),
              ('SpectralCrestFactorPerBand', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Sqr', {})
        dataflow_safe_append(df, 'SpectralCrestFactorPerBand', params)
        return df


class AutoCorrelation(AudioFeature):

    """
    Compute autocorrelation coefficients *ac* on each frames.

    .. math:: ac(k) = \sum_{i=0}^{N-k-1}x(i)x(i+k)

    """
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}),
              ('AutoCorrelation', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df, 'AutoCorrelation', params)
        return df


class LPC(AudioFeature):

    """
    Compute the Linear Predictor Coefficients (LPC) of a signal frame.
    It uses autocorrelation and Levinson-Durbin algorithm. see [JM1975]_.

    .. [JM1975] Makoul J., *Linear Prediction: A tutorial Review*,
                Proc. IEEE, Vol. 63, pp. 561-580, 1975.

    """
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(AutoCorrelation, {'ACNbCoeffs': None}),
              ('AC2LPC', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        acparams = AutoCorrelation.filter_params(params)
        acparams['ACNbCoeffs'] = str(int(params.get('LPCNbCoeffs')) + 1)
        df = AutoCorrelation.get_dataflow(acparams, samplerate)
        dataflow_safe_append(df, 'AC2LPC', params)
        return df


class LSF(AudioFeature):

    """
    Compute the Line Spectral Frequency (LSF) coefficients of a signal
    frame. Algorithm was adapted from ([TB2006]_, [SH1976]_).

    .. [TB2006] Tom Backstrom, Carlo Magi, *Properties of line spectrum
                pair polynomials--A review*, Signal Processing, Volume 86,
                Issue 11, Special Section: Distributed Source Coding,
                November 2006, Pages 3286-3298, ISSN 0165-1684,
                DOI: 10.1016/j.sigpro.2006.01.010.
    .. [SH1976] Schussler, H., *A stability theorem for discrete systems*,
                Acoustics, Speech and Signal Processing, IEEE Transactions
                on , vol.24, no.1, pp. 87-89, Feb 1976
    """
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(LPC, {'LPCNbCoeffs': None}),
              ('LPC2LSF', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        nbCoeffs = int(params.get('LSFNbCoeffs'))
        displacement = int(params.get('LSFDisplacement'))
        lpcparams = LPC.filter_params(params)
        lpcparams['LPCNbCoeffs'] = nbCoeffs + 1 - max(displacement, 1)
        df = LPC.get_dataflow(lpcparams, samplerate)
        dataflow_safe_append(df, 'LPC2LSF', params)
        return df


class AutoCorrelationPeaksIntegrator(AudioFeature):

    '''
    Feature transform that compute peaks of the autocorrelation function,
    outputs peaks and amplitude.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('AutoCorrelationPeaksIntegrator', {})]
    TRANSFORM = True

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        dataflow_safe_append(df, 'AutoCorrelationPeaksIntegrator', params)
        return df


class ComplexDomainOnsetDetection(AudioFeature):

    '''
    Compute onset detection using a complex domain spectral flux
    method [CD2003]_.

    .. [CD2003] C.Duxbury et al., *Complex domain onset detection for musical
                signals*, Proc. of the 6th Int. Conference on Digital Audio
                Effects (DAFx-03), London, UK, September 8-11, 2003
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}),
              ('FFT', {}),
              ('ComplexDomainFlux', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df, 'FFT', params)
        dataflow_safe_append(df, 'ComplexDomainFlux', params)
        return df


class Cepstrum(AudioFeature):

    '''
    Feature transform that compute cepstrum coefficients of input feature
    frames.  (use DCT II)

    .. math:: cep = dct(log(x))
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('Cepstrum', {})]
    TRANSFORM = True

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        dataflow_safe_append(df, 'Cepstrum', params)
        return df


class MelSpectrum(AudioFeature):

    '''
    Compute the Mel-frequencies spectrum [DM1980]_.

    Mel filter bank is built as 40 log-spaced filters according to the
    following mel-scale:

    .. math:: melfreq = 1127 * log(1 + \\frac{freq}{700})

    Each filter is a triangular filter with height :math:`2/(f_{max}-f_{min})`.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {'FFTLength': None}),
              ('MelFilterBank', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(
            MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df, 'MelFilterBank', params)
        return df


class MFCC(AudioFeature):

    '''
    Compute the Mel-frequencies cepstrum coefficients [DM1980]_.

    Mel filter bank is built as 40 log-spaced filters according to
    the following mel-scale:

    .. math:: melfreq = 1127 * log(1 + \\frac{freq}{700})

    Each filter is a triangular filter with height :math:`2/(f_{max}-f_{min})`.
    Then MFCCs are computed as following, using DCT II:

    .. math:: mfcc = dct(log(abs(fft(hanning(N).x)).MelFilterBank))

    .. [DM1980] S.B. Davis and P.Mermelstrin, *Comparison of parametric
                representations for monosyllabic word recognition in
                continuously spoken sentences.* IEEE Transactions on
                Acoustics, Speech and Signal Processing,
                28 :357-366, 1980.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {'FFTLength': None}),
              ('MelFilterBank', {}),
              ('Cepstrum', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(
            MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df, 'MelFilterBank', params)
        dataflow_safe_append(df, 'Cepstrum', params)
        return df


class Derivate(AudioFeature):

    '''
    Compute temporal derivative of input feature. The derivative is
    approximated by an orthogonal polynomial fit over a finite length
    window. (see [RR1993]_ p.117).

    .. math::

        \\frac{\partial x(t)}{\partial t} = \mu \sum_{k=-N}^{N}k.x(t+k)

        where \: \mu = \sum_{k=-N}^{N}k^2


    .. [RR1993] L.R.Rabiner, *Fundamentals of Speech Processing*.
                Prentice Hall Signal Processing Series.
                PTR Prentice-Hall, 1993.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('Derivate', {})]
    TRANSFORM = True

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('Derivate', params)
        return df


class Envelope(AudioFeature):

    '''
    Extract amplitude envelope using hilbert transform, low-pass filtering and
    decimation.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {'blockSize': '32768', 'stepSize': '16384'}),
              ('Envelope', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df, 'Envelope', params)
        return df


class EnvelopeShapeStatistics(AudioFeature):

    '''
    Centroid, spread, skewness and kurtosis of each frame's amplitude envelope.
    For more details about moments,
    see :ref:`Shape Statistics <shapestatistics>`.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Envelope, {}),
              ('ShapeStatistics', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Envelope.get_dataflow(Envelope.filter_params(params), samplerate)
        dataflow_safe_append(df, 'ShapeStatistics', params)
        return df


class AmplitudeModulation(AudioFeature):

    '''
    Tremelo and Grain description, according to [SE2005]_ and [AE2001]_.

    AmplitudeModulation uses :class:`Envelope` to describe tremolo and grain.
    Analyzed frequency ranges are :

     * Tremolo : 4 - 8 Hz
     * Grain : 10 - 40 Hz

    For each of these ranges, it computes :

     #. Frequency of maximum energy in range
     #. Difference of the energy of this frequency and the mean energy over all
        frequencies
     #. Difference of the energy of this frequency and the mean energy in range
     #. Product of the two first values.

    .. [AE2001] A.Eronen, *Automatic musical instrument recognition.* Master's
                Thesis, Tempere University of Technology, 2001.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Envelope, {}),
              ('AmplitudeModulation', {'EnDecim': None})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Envelope.get_dataflow(Envelope.filter_params(params), samplerate)
        dataflow_safe_append(df, 'AmplitudeModulation', params)
        return df


class StatisticalIntegrator(AudioFeature):

    '''
    Feature transform that compute the temporal mean and variance
    of input feature over the given number of frames.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    TRANSFORM = True
    PARAMS = [('StatisticalIntegrator', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('StatisticalIntegrator', params)
        return df


class HistogramIntegrator(AudioFeature):

    '''
    Feature transform that compute histogram of input values
    '''
    COMPONENT_LIBS = ['yaafe-components']
    TRANSFORM = True
    PARAMS = [('HistogramIntegrator', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('HistogramIntegrator', params)
        return df


class SlopeIntegrator(AudioFeature):

    '''
    Feature transform that compute the slope of input feature over the given
    number of frames.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    TRANSFORM = True
    PARAMS = [('SlopeIntegrator', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('SlopeIntegrator', params)
        return df


class Loudness(AudioFeature):

    '''
    The loudness coefficients are the energy in each Bark band, normalized by
    the overall sum. see [GP2004]_ and [MG1997]_ for more details.

    .. [MG1997] Moore, Glasberg, et al., *A Model for the Prediction of
                Thresholds Loudness and Partial Loudness.*,
                J. Audio Eng. Soc. 45: 224-240, 1997.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Sqr', {}),
              ('Loudness', {}),
              ('LMode', 'Relative',
               '"Specific" computes loudness without normalization, '
               '"Relative" normalize each band so that they sum to 1, '
               '"Total" just returns the sum of Loudness in all bands.')]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(
            MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df, 'Sqr', {})
        dataflow_safe_append(df, 'Loudness', params)
        if (params['LMode'] == 'Relative'):
            dataflow_safe_append(df, 'Normalize', {'NNorm': 'Sum'})
        elif (params['LMode'] == 'Total'):
            dataflow_safe_append(df, 'Sum', {})
        return df


class PerceptualSharpness(AudioFeature):

    '''
    Compute the sharpness of :class:`Loudness` coefficients,
    according to [GP2004]_.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Loudness, {'LMode': None}),
              ('LoudnessSharpness', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        lParams = Loudness.filter_params(params)
        lParams['LMode'] = 'Relative'
        df = Loudness.get_dataflow(lParams, samplerate)
        dataflow_safe_append(df, 'LoudnessSharpness', {})
        return df


class PerceptualSpread(AudioFeature):

    '''
    Compute the spread of :class:`Loudness` coefficients,
    according to [GP2004]_.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Loudness, {'LMode': None}),
              ('LoudnessSpread', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        lParams = Loudness.filter_params(params)
        lParams['LMode'] = 'Relative'
        df = Loudness.get_dataflow(lParams, samplerate)
        dataflow_safe_append(df, 'LoudnessSpread', {})
        return df


class OBSI(AudioFeature):

    '''
    Compute Octave band signal intensity using a trigular octave
    filter bank ([SE2005]_).

    .. [SE2005] S.Essid, *Classification automatique des signaux
                audio-frequences: reconnaissance des instruments
                de musique*. PhD, UPMC, 2005.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('Sqr', {}),
              ('OBSI', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(
            MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df, 'Sqr', {})
        dataflow_safe_append(df, 'OBSI', params)
        return df


class OBSIR(AudioFeature):

    '''
    Compute log of :class:`OBSI` ratio between consecutive octave.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(OBSI, {}),
              ('Difference', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = OBSI.get_dataflow(OBSI.filter_params(params), samplerate)
        dataflow_safe_append(df, 'Difference', params)
        return df


class OnsetDetectionFunction(AudioFeature):

    '''
    Compute onset detection function (spectral energy flux)
    according to [MA2005]_ method.

    .. [MA2005] M.Alonso, G.Richard, B.David,
    *EXTRACTING NOTE ONSETS FROM MUSICAL RECORDINGS*,
    International Conference on Multimedia and Expo (IEEE-ICME'05), Amsterdam,
    The Netherlands, 2005.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum, {}),
              ('NormalizeMaxAll', {'NMANbFrames': '5000'})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(
            MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df, 'NormalizeMaxAll', params)
        dataflow_safe_append(
            df, 'FilterSmallValues', {'FSVThreshold': '0.001'})
        dataflow_safe_append(df, 'HalfHannFilter', {'HHFOrder': '0.175s'})
        dataflow_safe_append(df, 'LogCompression', {})
        dataflow_safe_append(
            df, 'DvornikovDifferentiator', {'DDOrder': '0.08s'})
        dataflow_safe_append(df, 'FilterSmallValues', {'FSVThreshold': '1'})
        dataflow_safe_append(df, 'Sum', {})
        dataflow_safe_append(df, 'NormalizeMaxAll', params)
        return df


class BeatHistogramSummary(AudioFeature):

    '''
    Compute the beat histogram according to [GT2002]_,
    but using :class:`OnsetDetectionFunction` as onset detection function.

    .. [GT2002] Georges Tzanetakis,
    *Musical Genre Classification of Audio Signals*,
    IEEE Transactions on speech and audio processing, vol. 10, No. 5, July 2002.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(OnsetDetectionFunction, {}),
              ('AutoCorrelationPeaksIntegrator', {'NbFrames': None,
                                                  'StepNbFrames': None,
                                                  'ACPNbPeaks': '3',
                                                  'ACPNorm': None,
                                                  'ACPInterPeakMinDist': None}),
              ('HistogramIntegrator', {'NbFrames': None,
                                       'StepNbFrames': None,
                                       'HInf': None,
                                       'HSup': None,
                                       'HNbBins': '80',
                                       'HWeighted': None}),
              ('HistogramSummary', {'HSup': None, 'HInf': None}),
              ('BHSBeatFrameSize', '128',
               'Number of frames over which autocorrelation peaks is computed'),
              ('BHSBeatFrameStep', '64',
               'Number of frames to skip between two consecutive autocorrelation peaks computation'),
              ('BHSHistogramFrameSize', '40',
               'Number of beat frames over which histogram is computed'),
              ('BHSHistogramFrameStep', '40',
               'Number of beat frames to skip between two consecutive histogram computation'),
              ('HInf', '40', 'Minimal BPM to take into consideration'),
              ('HSup', '200', 'Maximal BPM to tage into consideration')]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = OnsetDetectionFunction.get_dataflow(
            OnsetDetectionFunction.filter_params(params), samplerate)
        dataflow_safe_append(df, 'AutoCorrelationPeaksIntegrator',
                             {'NbFrames': params['BHSBeatFrameSize'],
                              'StepNbFrames': params['BHSBeatFrameStep'],
                              'ACPNbPeaks': params['ACPNbPeaks'],
                              'ACPNorm': 'BPM',
                              'ACPInterPeakMinDist': '5'})
        dataflow_safe_append(df, 'HistogramIntegrator',
                             {'NbFrames': params['BHSHistogramFrameSize'],
                              'StepNbFrames': params['BHSHistogramFrameStep'],
                              'HInf': params['HInf'],
                              'HSup': params['HSup'],
                              'HNbBins': params['HNbBins'],
                              'HWeighted': '1'})
        dataflow_safe_append(df, 'HistogramSummary', params)
        return df


class CQT(AudioFeature):

    '''
    Compute the Constant-Q transform according to [CS2010]_
    with improvements from [JPCQT]_.

    .. [CS2010] C.Schörkhuber and A.Klapuri, *CONSTANT-Q TRANSFORM TOOLBOX FOR MUSIC PROCESSING*, 7th Sound and Music Conference (SMC'2010), 2010, Barcelona.
    .. [JPCQT] J.Prado, *Calcul rapide de la transformée à Q constant*, http://perso.telecom-paristech.fr/~prado/cqt/cqt_modif.pdf
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {'blockSize': None}),
              ('CQT', {'CQTMinFreq': '73.42', 'CQTBinsPerOctave': '36',
               'CQTMaxFreq': None}),
              ('CQTNbOctaves', '3',
               'Number of octaves to consider for analysis')]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        nboct = decimal.Decimal(params['CQTNbOctaves'])
        original_fs = samplerate
        original_step = int(params['stepSize'])
        samplerate = decimal.Decimal(samplerate)
        minFreq = decimal.Decimal(params['CQTMinFreq']) * (2 ** (nboct - 1))
        bandwidth = decimal.Decimal('0.95')
        # create dataflow
        df = DataFlow()
        signal_node = None
        # decimate until first octave to analyse
        nbdecim = 0
        while ((bandwidth * samplerate / 8) > minFreq):
            s = df.createNode('Decimate2', {})
            if signal_node:
                df.link(signal_node, '', s, '')
            signal_node = s
            samplerate = samplerate / 2
            nbdecim = nbdecim + 1
        # check stepsize is OK with decimation
#        stepSize = int(params['stepSize'])
#        if (stepSize % (2**(nbdecim+nboct-1)))!=0:
#            stepSize = stepSize - stepSize % (2**(nbdecim+nboct-1))
#            print('WARNING: adjust stepSize to %i to make it compatible with successive decimation'%stepSize)
        # compute octave CQT parameters
        Q = 2 / (2 ** (1.0 / int(params['CQTBinsPerOctave'])) - 1)
        fftLen = Q * float(samplerate / minFreq)
        fftLen = pow(2, math.ceil(math.log(fftLen) / math.log(2)))
#        current_stepSize = stepSize / (2**nbdecim)
        oct_params = {'CQTBinsPerOctave': params['CQTBinsPerOctave'],
                      'CQTAlign': params['CQTAlign'],
                      'CQTMinFreq': '%s' % str(minFreq / samplerate),
                      'CQTMaxFreq': '%s' % str(2 * minFreq / samplerate - decimal.Decimal('1e-14'))}
        # for each octave, analysis, concatenate and decimation
        concat_node = df.createNode('Concatenate', {})
        for oct in range(nboct, 0, -1):
            frames = df.createNode('AdvancedFrameTokenizer',
                                   {'blockSize': '%i' % fftLen,
                                    'outStepSize': '%i' % original_step,
                                    'outSampleRate': '%f' % original_fs})
            if signal_node:
                df.link(signal_node, '', frames, '')
            cspec = df.createNode(
                'FFT', {'FFTLength': '%i' % fftLen, 'FFTWindow': 'None'})
            df.link(frames, '', cspec, '')
            oct_cq = df.createNode('CQT', oct_params)
            df.link(cspec, '', oct_cq, '')
            df.link(oct_cq, '', concat_node, '%i' % (oct - 1))
            if (oct == 1):
                # no more octave to analyze, no need to decimate any more
                break
            # decimation for next actave analysis
            s = df.createNode('Decimate2', {})
            if signal_node:
                df.link(signal_node, '', s, '')
            signal_node = s
#            current_stepSize = current_stepSize / 2
            minFreq = minFreq / 2
        return df


class CQT2(AudioFeature):

    '''
    Compute the Constant-Q transform
    according to Blankertz's implementation [BB]_, with improvments
    from [JP2010]_.

    .. [BB] B.Blankertz, *The Constant Q Transform*, http://wwwmath.uni-muenster.de/logik/Personen/blankertz/constQ/constQ.html
    .. [JP2010] J.Prado, *Transformée à Q constant*, technical report 2010D004, Institut TELECOM, TELECOM ParisTech, CNRS LTCI, 2010.'''

    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {'blockSize': None}),
              ('CQT', {})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        minFreq = float(params['CQTMinFreq'])
        bins = int(params['CQTBinsPerOctave'])
        Q = 2.0 / (pow(2.0, 1.0 / bins) - 1)
        fftLen = Q * samplerate / minFreq
        fftLen = pow(2, math.ceil(math.log(fftLen) / math.log(2)))
        fParams = Frames.filter_params(params)
        fParams['blockSize'] = '%i' % fftLen
        df = Frames.get_dataflow(fParams, samplerate)
        dataflow_safe_append(
            df, 'FFT', {'FFTLength': '%i' % fftLen, 'FFTWindow': 'None'})
        dataflow_safe_append(df, 'CQT', params)
        return df


class SpectralIrregularity(AudioFeature):

    '''
    Compute difference between consecutive CQT bins, see [Brown2000]_.

    .. [Brown2000] J.C. Brown, O.Houix, Stephen McAdams, *Feature dependence in the automatic identification of musical woodwind instruments.*, Journal of the Acoustical Society of America, 109: 1064-1072, 2000.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(CQT, {}),
              ('Difference', {'DiffNbCoeffs': None})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = CQT.get_dataflow(params, samplerate)
        dataflow_safe_append(df, 'Difference', {'DiffNbCoeffs': '0'})
        return df


class Chroma2(AudioFeature):

    '''
    Chroma2 compute short-term pitch profile according to [ZK2006]_.

    .. [ZK2006] Y. Zhu and M.S. Kankanhalli. *Precise pitch profile feature extraction from musical audio for key detection.* IEEE Transactions on Multimedia, 2006.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(CQT, {'CQTMinFreq': '27.5',
                     'CQTNbOctaves': '7',
                     'CQTBinsPerOctave': '48'}),
              ('Chroma2', {'CQTMinFreq': None,
                           'CQTMaxFreq': None,
                           'CQTBinsPerOctave': None})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        tuning = float(params['CZTuning'])
        fmin = float(params['CQTMinFreq'])
        if tuning > 0:
            # adjust min freq to a divisor of tuning
            b = int(params['CQTBinsPerOctave'])
            if (b % 12 != 0):
                print('WARNING: in Chroma2, CQTBinsPerOctave must be multiple of 12')
                b = b - b % 12
                if b == 0:
                    b = 12
                print('use CQTBinsPerOctave=%i' % b)
            dev = b * math.log(tuning / fmin) / math.log(2)
            fmin *= pow(2.0, math.fmod(dev, 1) / b)
        params['CQTMinFreq'] = str(fmin)
        df = CQT.get_dataflow(CQT.filter_params(params), samplerate)
        chParams = params
        del chParams['stepSize']
        dataflow_safe_append(df, 'Chroma2', chParams)
        return df


class Chroma(AudioFeature):

    '''
    Chroma compute short-term chromagram according to [BP2005]_.

    .. [BP2005] Bello, J.P. and Pickens, J. *A Robust Mid-level Representation for Harmonic Content in Music Signals.* In Proceedings of the 6th International Conference on Music Information Retrieval (ISMIR-05), London, UK. September 2005.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('ChromaSmoothing', '0.75s', 'Chroma smoothing duration'),
              (CQT, {'CQTMinFreq': '73.42', 'CQTNbOctaves': '3',
                     'CQTBinsPerOctave': '36'}),
              ('ChromaTune', {'CQTMinFreq': None, 'CQTBinsPerOctave': None,
                              'CTInitDuration': '15'})]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        b = int(params['CQTBinsPerOctave'])
        if (b % 12 != 0):
            print('WARNING: in Chroma, CQTBinsPerOctave must be multiple of 12')
            b = b - b % 12
            if b == 0:
                b = 12
            print('use CQTBinsPerOctave=%i' % b)
            params['CQTBinsPerOctave'] = b
        df = CQT.get_dataflow(CQT.filter_params(params), samplerate)
        dataflow_safe_append(df, 'ChromaTune', params)
        dataflow_safe_append(
            df, 'MedianFilter', {'MFOrder': params['ChromaSmoothing']})
        dataflow_safe_append(df, 'ChromaReduce', {})
        return df


class Chords(AudioFeature):

    '''
    Chords recognize chords from chromagrams, according to L.Oudre's algorithm [LO2011]_.

    .. [LO2011] Oudre, L. and Grenier, Y. and Fevotte, C., *Chord recognition by fitting rescaled chroma vectors to chord templates*, IEEE Transactions on Audio, Speech and Language Processing, vol. 19, pages 2222 - 2233, Sep. 2011.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('FrameTokenizer', {'blockSize': None}),
              ('ChordsUse7', '0',
               'If 1 then use 7th chords to enrich chord dictionnary, else use only major an minor chords'),
              ('ChordsSmoothing', '1.5s', 'Chords smoothing duration')]

    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        if (params['ChordsUse7'] == '1'):
            chtype = 'maj,min,7'
        else:
            chtype = 'maj,min'
        df = Chroma.get_dataflow({'CQTMinFreq': '73.42', 'CQTNbOctaves': '3',
                                  'CQTBinsPerOctave': '36',
                                  'stepSize': params['stepSize']},
                                 samplerate)
        dataflow_safe_append(df, 'Chroma2ChordDict', {'ChordTypes': chtype,
                                                      'ChordNbHarmonics': '1'})
        dataflow_safe_append(df, 'MedianFilter',
                             {'MFOrder': params['ChordsSmoothing']})
        dataflow_safe_append(df, 'ChordDictDecoder', {'ChordTypes': chtype})
        return df
