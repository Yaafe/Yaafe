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
from yaafelib import AudioFeature, check_dataflow_params, dataflow_safe_append, DataFlow

class Frames(AudioFeature):
    '''
    Segment input signal into frames.
    
    First frame has zeros on left half so that it is centered on time 0s, then consecutive frames are equally spaced.
    Consequently, frame *i* (starting from 0) is centered on sample *i* * *stepSize*. 
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('FrameTokenizer',{}),]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = DataFlow()
        df.createNode('FrameTokenizer',params)
        return df

class ZCR(AudioFeature):
    '''
    Compute zero-crossing rate in frames. see [SS1997]_.
    
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames,{}),]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params),samplerate)
        dataflow_safe_append(df,'ZCR',{})
        return df

class TemporalShapeStatistics(AudioFeature):
    '''
    Compute :ref:`shape statistics <shapestatistics>` of signal frames.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames,{}),]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params),samplerate)
        dataflow_safe_append(df,'ShapeStatistics',{})
        return df

class Energy(AudioFeature):
    '''
    Compute energy as root mean square of an audio Frame.
    
    .. math:: en = \sqrt\\frac{\sum_{i=0}^{N-1}x(i)^2}{N}
    
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {}),]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params),samplerate)
        dataflow_safe_append(df,'RMS',{})
        return df
        

class MagnitudeSpectrum(AudioFeature):
    '''
    Compute frame's magnitude spectrum, using an analysis window (Hanning or Hamming), or not.
    
    .. math:: y = abs(fft(w*x))
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames,{}),
              ('FFT',{'FFTLength':'0'})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        if (params['FFTLength']=='0'):
            params['FFTLength']=params['blockSize']
        dataflow_safe_append(df,'FFT',params)
        dataflow_safe_append(df,'Abs',{})
        return df

class SpectralVariation(AudioFeature):
    '''
    SpectralVariation is the normalized correlation of :class:`spectrum <MagnitudeSpectrum>` between consecutive frames. (see [GP2004]_)
    
    .. math:: S_{var} = 1 - \\frac{\sum_{k}a_{k}(t-1)a_{k}(t)}{\sqrt{\sum_{k}a_{k}(t-1)^2}\sqrt{\sum_{k}a_{k}(t)^2}}
    
    .. [GP2004] Geoffroy Peeters, *A large set of audio features for sound description (similarity and classification) in the CUIDADO project*, 2004.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Variation',params)
        return df

class SpectralSlope(AudioFeature):
    '''
    SpectralSlope is computed by linear regression of the spectral amplitude. (see [GP2004]_)
    
    .. math:: S_{slope} = \\frac{ K\sum_{k}f_{k}a_{k} - \sum_{k}f_{k}\sum_{k}a_{k}}{K\sum_{k}f_{k}^2-(\sum_{k}a_{k})^2}
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Slope',params)
        return df
    

class SpectralRolloff(AudioFeature):
    '''
    Spectral roll-off is the frequency so that 99% of the energy is contained below. see [SS1997]_.

    .. [SS1997] E.Scheirer, M.Slaney. *Construction and evaluation of a robust multifeature speech/music discriminator*. IEEE Internation Conference on Acoustics, Speech and Signal Processing, p.1331-1334, 1997.     

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Rolloff',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Sqr',{})
        dataflow_safe_append(df,'Rolloff',params)
        return df

class SpectralShapeStatistics(AudioFeature):
    '''
    Compute shape statistics of :class:`MagnitudeSpectrum`, (see [GR2004]_).
    
    .. _shapestatistics:
    
    **Shape Statistics** are centroid, spread, skewness and kurtosis, defined as follow:    
    
    .. math::
    
        \mu_{i} &= \\frac{\sum_{n=1}^{N}f_{k}^{i}*a_{k}}{\sum_{n=1}^{N}a_{k}}\\\\
        centroid &= \mu_{1}\\\\
        spread &= \sqrt{\mu_{2}-\mu_{1}^{2}} \\\\ 
        skewness &= \\frac{2\mu_{1}^{3} - 3\mu_{1}\mu_{2} + \mu_{3}}{spread^{3}} \\\\ 
        kurtosis &= \\frac{-3\mu_{1}^{4} + 6\mu_{1}\mu_{2} - 4\mu_{1}\mu_{3} + \mu_{4}}{spread^{4}} - 3
        
    .. [GR2004] O.Gillet, G.Richard, *Automatic transcription of drum loops.* in IEEE International Conference on Acoustics, Speech and Signal Processing (ICASSP), Montreal, Canada, 2004.
    
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('ShapeStatistics',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'ShapeStatistics',params)
        return df

class SpectralFlux(AudioFeature):
    '''
    Compute flux of :class:`spectrum <MagnitudeSpectrum>` between consecutives frames.
    
    .. math:: S_{flux} = \\frac{\sum_{k}(a_{k}(t) - a_{k}(t-1))^2}{\sqrt{\sum_{k}a_{k}(t-1)^2}\sqrt{\sum_{k}a_{k}(t)^2}}
    
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Flux',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df,'Flux',params)
        return df

class SpectralFlatness(AudioFeature):
    '''
    Compute global spectral flatness using the ratio between geometric and arithmetic mean.
    
    .. math:: S_{flatness} = \\frac{exp(\\frac{1}{N}\sum_{k}log(a_{k}))}{\\frac{1}{N}\sum_{k}a_{k}}
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Flatness',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Flatness',params)
        return df


class SpectralDecrease(AudioFeature):
    '''
    Compute spectral decrease accoding to [GP2004]_.
    
    .. math:: S_{decrease} = \\frac{1}{\sum_{k=2}^{K}a_{k}}\sum_{k=2}^{K}\\frac{a_{k}-a_{1}}{k-1}
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Decrease',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Decrease',params)
        return df

class SpectralFlatnessPerBand(AudioFeature):
    '''
    Compute spectral flatness per log-spaced band of 1/4 octave, as proposed in MPEG7 standard.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Sqr',{}),
              ('SpectralFlatnessPerBand',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Sqr',{})
        dataflow_safe_append(df,'SpectralFlatnessPerBand',params)
        return df

class SpectralCrestFactorPerBand(AudioFeature):
    '''
    Compute spectral crest factor per log-spaced band of 1/4 octave.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Sqr',{}),
              ('SpectralCrestFactorPerBand',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(params, samplerate)
        dataflow_safe_append(df,'Sqr',{})
        dataflow_safe_append(df,'SpectralCrestFactorPerBand',params)
        return df

class AutoCorrelation(AudioFeature):
    """
    Compute autocorrelation coefficients *ac* on each frames.
    
    .. math:: ac(k) = \sum_{i=0}^{N-k-1}x(i)x(i+k)
    
    """
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames,{}),
              ('AutoCorrelation',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df,'AutoCorrelation',params)
        return df

class LPC(AudioFeature):
    """
    Compute the Linear Predictor Coefficients (LPC) of a signal frame. It uses autocorrelation and Levinson-Durbin algorithm. see [JM1975]_.
    
    .. [JM1975] Makoul J., *Linear Prediction: A tutorial Review*, Proc. IEEE, Vol. 63, pp. 561-580, 1975.
    
    """
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(AutoCorrelation,{'ACNbCoeffs':None}),
              ('AC2LPC',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params, samplerate):
        acparams = AutoCorrelation.filter_params(params)
        acparams['ACNbCoeffs'] = str(int(params.get('LPCNbCoeffs')) + 1)
        df = AutoCorrelation.get_dataflow(acparams, samplerate)
        dataflow_safe_append(df,'AC2LPC',params)
        return df

class LSF(AudioFeature):
    """
    Compute the Line Spectral Frequency (LSF) coefficients of a signal frame. Algorithm was adapted from ([TB2006]_, [SH1976]_).

    .. [TB2006] Tom Backstrom, Carlo Magi, *Properties of line spectrum pair polynomials--A review*, Signal Processing, Volume 86, Issue 11, Special Section: Distributed Source Coding, November 2006, Pages 3286-3298, ISSN 0165-1684, DOI: 10.1016/j.sigpro.2006.01.010.
    .. [SH1976] Schussler, H., *A stability theorem for discrete systems*, Acoustics, Speech and Signal Processing, IEEE Transactions on , vol.24, no.1, pp. 87-89, Feb 1976
    """
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(LPC,{'LPCNbCoeffs':None}),
              ('LPC2LSF',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params, samplerate):
        nbCoeffs = int(params.get('LSFNbCoeffs'))
        displacement = int(params.get('LSFDisplacement'))
        lpcparams = LPC.filter_params(params)
        lpcparams['LPCNbCoeffs'] = nbCoeffs + 1 - max(displacement,1)
        df = LPC.get_dataflow(lpcparams, samplerate)
        dataflow_safe_append(df,'LPC2LSF',params) 
        return df

class AutoCorrelationPeaksIntegrator(AudioFeature):
    '''
    Feature transform that compute peaks of the autocorrelation function, outputs peaks and amplitude.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('AutoCorrelationPeaksIntegrator',{})]
    TRANSFORM = True
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params, samplerate):
        df = DataFlow()
        dataflow_safe_append(df,'AutoCorrelationPeaksIntegrator',params)
        return df

class ComplexDomainOnsetDetection(AudioFeature):
    '''
    Compute onset detection using a complex domain spectral flux method [CD2003]_.
    
    .. [CD2003] C.Duxbury et al., *Complex domain onset detection for musical signals*, Proc. of the 6th Int. Conference on Digital Audio Effects (DAFx-03), London, UK, September 8-11, 2003
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames,{}),
              ('FFT',{}),
              ('ComplexDomainFlux',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df,'FFT',params)
        dataflow_safe_append(df,'ComplexDomainFlux',params)
        return df

class Cepstrum(AudioFeature):
    '''
    Feature transform that compute cepstrum coefficients of input feature frames. (use DCT II)
    
    .. math:: cep = dct(log(x))
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('Cepstrum',{})]
    TRANSFORM = True
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = DataFlow()
        dataflow_safe_append(df,'Cepstrum',params)
        return df

class MelSpectrum(AudioFeature):
    '''
    Compute the Mel-frequencies spectrum [DM1980]_.
    
    Mel filter bank is built as 40 log-spaced filters according to the following mel-scale:
    
    .. math:: melfreq = 1127 * log(1 + \\frac{freq}{700})
    
    Each filter is a triangular filter with height :math:`2/(f_{max}-f_{min})`.
    
    .. [DM1980] S.B. Davis and P.Mermelstrin, *Comparison of parametric representations for monosyllabic word recognition in continuously spoken sentences.* IEEE Transactions on Acoustics, Speech and Signal Processing, 28 :357-366, 1980.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{'FFTLength':None}),
              ('MelFilterBank',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df,'MelFilterBank',params)
        return df

class MFCC(AudioFeature):
    '''
    Compute the Mel-frequencies cepstrum coefficients [DM1980]_.
    
    Mel filter bank is built as 40 log-spaced filters according to the following mel-scale:
    
    .. math:: melfreq = 1127 * log(1 + \\frac{freq}{700})
    
    Each filter is a triangular filter with height :math:`2/(f_{max}-f_{min})`.
    Then MFCCs are computed as following, using DCT II:
    
    .. math:: mfcc = dct(log(abs(fft(hanning(N).x)).MelFilterBank))
    
    .. [DM1980] S.B. Davis and P.Mermelstrin, *Comparison of parametric representations for monosyllabic word recognition in continuously spoken sentences.* IEEE Transactions on Acoustics, Speech and Signal Processing, 28 :357-366, 1980.

    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{'FFTLength':None}),
              ('MelFilterBank',{}),
              ('Cepstrum',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls,params,samplerate):
        df = MagnitudeSpectrum.get_dataflow(MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df,'MelFilterBank',params)
        dataflow_safe_append(df,'Cepstrum',params)
        return df

class Derivate(AudioFeature):
    '''
    Compute temporal derivative of input feature. The derivative is approximated by 
    an orthogonal polynomial fit over a finite length window. (see [RR1993]_ p.117).
    
    .. math:: 
    
        \\frac{\partial x(t)}{\partial t} = \mu \sum_{k=-N}^{N}k.x(t+k)
        
        where \: \mu = \sum_{k=-N}^{N}k^2
    
    
    .. [RR1993] L.R.Rabiner, *Fundamentals of Speech Processing*. Prentice Hall Signal Processing Series. PTR Prentice-Hall, 1993.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [('Derivate',{})]
    TRANSFORM = True
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('Derivate',params)
        return df

class Envelope(AudioFeature):
    '''
    Extract amplitude envelope using hilbert transform, low-pass filtering and decimation.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Frames, {'blockSize':'32768', 'stepSize':'16384'}),
              ('Envelope', {})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Frames.get_dataflow(Frames.filter_params(params), samplerate)
        dataflow_safe_append(df,'Envelope',params)
        return df

class EnvelopeShapeStatistics(AudioFeature):
    '''
    Centroid, spread, skewness and kurtosis of each frame's amplitude envelope. For more details about moments, see :ref:`Shape Statistics <shapestatistics>`.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Envelope,{}),
              ('ShapeStatistics',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Envelope.get_dataflow(Envelope.filter_params(params), samplerate)
        dataflow_safe_append(df,'ShapeStatistics',params)
        return df

class AmplitudeModulation(AudioFeature):
    '''
    Tremelo and Grain description, according to [SE2005]_ and [AE2001]_.
    
    AmplitudeModulation uses :class:`Envelope` to describe tremolo and grain. Analyzed frequency ranges are :
     * Tremolo : 4 - 8 Hz
     * Grain : 10 - 40 Hz

    For each of these ranges, it computes :
     #. Frequency of maximum energy in range
     #. Difference of the energy of this frequency and the mean energy over all frequencies
     #. Difference of the energy of this frequency and the mean energy in range
     #. Product of the two first values.
     
    .. [AE2001] A.Eronen, *Automatic musical instrument recognition.* Master's Thesis, Tempere University of Technology, 2001.
    
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Envelope,{}),
              ('AmplitudeModulation',{'EnDecim':None})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = Envelope.get_dataflow(Envelope.filter_params(params), samplerate)
        dataflow_safe_append(df,'AmplitudeModulation',params)
        return df
  
class StatisticalIntegrator(AudioFeature):
    '''
    Feature transform that compute the temporal mean and variance of input feature over the given number of frames.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    TRANSFORM = True
    PARAMS = [('StatisticalIntegrator',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('StatisticalIntegrator',params)
        return df

class HistogramIntegrator(AudioFeature):
    '''
    Feature transform that compute histogram of input values
    '''
    COMPONENT_LIBS = ['yaafe-components']
    TRANSFORM = True
    PARAMS = [('HistogramIntegrator',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('HistogramIntegrator',params)
        return df

class SlopeIntegrator(AudioFeature):
    '''
    Feature transform that compute the slope of input feature over the given number of frames.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    TRANSFORM = True
    PARAMS = [('SlopeIntegrator',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = DataFlow()
        df.createNode('SlopeIntegrator',params)
        return df

class Loudness(AudioFeature):
    '''
    The loudness coefficients are the energy in each Bark band, normalized by the overall sum. see [GP2004]_ and [MG1997]_ for more details.
    
    .. [MG1997] Moore, Glasberg, et al., *A Model for the Prediction of Thresholds Loudness and Partial Loudness.*, J. Audio Eng. Soc. 45: 224-240, 1997.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Sqr',{}),
              ('Loudness',{}),
              ('LMode','Relative','"Specific" computes loudness without normalization, "Relative" normalize each band so that they sum to 1, "Total" just returns the sum of Loudness in all bands.')]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df,'Sqr',{})
        dataflow_safe_append(df,'Loudness',params)
        if (params['LMode']=='Relative'):
            dataflow_safe_append(df,'Normalize',{'NNorm':'Sum'})
        elif (params['LMode']=='Total'):
            dataflow_safe_append(df,'Sum',{})
        return df

class PerceptualSharpness(AudioFeature):
    '''
    Compute the sharpness of :class:`Loudness` coefficients, according to [GP2004]_.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Loudness,{'LMode':None}),
              ('LoudnessSharpness',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        lParams = Loudness.filter_params(params)
        lParams['LMode'] = 'Relative'
        df = Loudness.get_dataflow(lParams, samplerate)
        dataflow_safe_append(df,'LoudnessSharpness',{})
        return df
    
class PerceptualSpread(AudioFeature):
    '''
    Compute the spread of :class:`Loudness` coefficients, according to [GP2004]_.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(Loudness,{'LMode':None}),
              ('LoudnessSpread',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        lParams = Loudness.filter_params(params)
        lParams['LMode'] = 'Relative'
        df = Loudness.get_dataflow(lParams, samplerate)
        dataflow_safe_append(df,'LoudnessSpread',{})
        return df

class OBSI(AudioFeature):
    '''
    Compute Octave band signal intensity using a trigular octave filter bank ([SE2005]_).
    
    .. [SE2005] S.Essid, *Classification automatique des signaux audio-frequences: reconnaissance des instruments de musique*. PhD, UPMC, 2005.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(MagnitudeSpectrum,{}),
              ('Sqr',{}),
              ('OBSI',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = MagnitudeSpectrum.get_dataflow(MagnitudeSpectrum.filter_params(params), samplerate)
        dataflow_safe_append(df,'Sqr',{})
        dataflow_safe_append(df,'OBSI',params)
        return df
    
class OBSIR(AudioFeature):
    '''
    Compute log of :class:`OBSI` ratio between consecutive octave.
    '''
    COMPONENT_LIBS = ['yaafe-components']
    PARAMS = [(OBSI,{}),
              ('Difference',{})]
    @classmethod
    @check_dataflow_params
    def get_dataflow(cls, params, samplerate):
        df = OBSI.get_dataflow(OBSI.filter_params(params), samplerate)
        dataflow_safe_append(df,'Difference',params)
        return df
