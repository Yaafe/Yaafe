/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Author : Benoit Mathieu
 *
 * This file is part of Yaafe.
 *
 * Yaafe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Yaafe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "registration.h"

#include "yaafe-core/ComponentFactory.h"

#include "yaafe-components/audio/Abs.h"
#include "yaafe-components/audio/AC2LPC.h"
#include "yaafe-components/audio/AmplitudeModulation.h"
#include "yaafe-components/audio/AutoCorrelation.h"
#include "yaafe-components/audio/AutoCorrelationPeaksIntegrator.h"
#include "yaafe-components/audio/Cepstrum.h"
#include "yaafe-components/audio/ComplexDomainFlux.h"
#include "yaafe-components/audio/Decrease.h"
#include "yaafe-components/audio/Derivate.h"
#include "yaafe-components/audio/Difference.h"
#include "yaafe-components/audio/Envelope.h"
#include "yaafe-components/audio/FFT.h"
#include "yaafe-components/audio/FilterSmallValues.h"
#include "yaafe-components/audio/Flatness.h"
#include "yaafe-components/audio/Flux.h"
#include "yaafe-components/audio/FrameTokenizer.h"
#include "yaafe-components/audio/HalfHannFilter.h"
#include "yaafe-components/audio/HistogramIntegrator.h"
#ifdef WITH_LAPACK
#include "yaafe-components/audio/LPC2LSF.h"
#endif
#include "yaafe-components/audio/SpecificLoudness.h"
#include "yaafe-components/audio/LoudnessSharpness.h"
#include "yaafe-components/audio/LoudnessSpread.h"
#include "yaafe-components/audio/MedianFilter.h"
#include "yaafe-components/audio/MelFilterBank.h"
#include "yaafe-components/audio/Normalize.h"
#include "yaafe-components/audio/OBSI.h"
#include "yaafe-components/audio/RMS.h"
#include "yaafe-components/audio/Rolloff.h"
#include "yaafe-components/audio/ShapeStatistics.h"
#include "yaafe-components/audio/Slope.h"
#include "yaafe-components/audio/SlopeIntegrator.h"
#include "yaafe-components/audio/SpectralCrestFactorPerBand.h"
#include "yaafe-components/audio/SpectralFlatnessPerBand.h"
#include "yaafe-components/audio/Sqr.h"
#include "yaafe-components/audio/StatisticalIntegrator.h"
#include "yaafe-components/audio/Sum.h"
#include "yaafe-components/audio/Variation.h"
#include "yaafe-components/audio/ZCR.h"
#include "components/AdvancedFrameTokenizer.h"
#include "components/CQT.h"
#include "components/ChordDictDecoder.h"
#include "components/Chroma2ChordDict.h"
#include "components/ChromaReduce.h"
#include "components/ChromaTune.h"
#include "components/Chroma2.h"
#include "components/Concatenate.h"
#include "components/Decimate2.h"
#include "components/DvornikovDifferentiator.h"
#include "components/HistogramSummary.h"
#include "components/LogCompression.h"
#include "components/NormalizeMaxAll.h"

#include "yaafe-components/flow/Join.h"

using namespace YAAFE;

void registerYaafeComponents(void* componentFactory)
{
  ComponentFactory* factory = reinterpret_cast<ComponentFactory*>(componentFactory);

  factory->registerPrototype(new AC2LPC());
  factory->registerPrototype(new Abs());
  factory->registerPrototype(new AmplitudeModulation());
  factory->registerPrototype(new AutoCorrelation());
  factory->registerPrototype(new AutoCorrelationPeaksIntegrator());
  factory->registerPrototype(new Cepstrum());
  factory->registerPrototype(new ComplexDomainFlux());
  factory->registerPrototype(new Decrease());
  factory->registerPrototype(new Derivate());
  factory->registerPrototype(new Difference());
  factory->registerPrototype(new Envelope());
  factory->registerPrototype(new FFT());
  factory->registerPrototype(new FilterSmallValues());
  factory->registerPrototype(new Flatness());
  factory->registerPrototype(new Flux());
  factory->registerPrototype(new FrameTokenizer());
  factory->registerPrototype(new HalfHannFilter());
  factory->registerPrototype(new HistogramIntegrator());
#ifdef WITH_LAPACK
  factory->registerPrototype(new LPC2LSF());
#endif
  factory->registerPrototype(new SpecificLoudness());
  factory->registerPrototype(new LoudnessSharpness());
  factory->registerPrototype(new LoudnessSpread());
  factory->registerPrototype(new MedianFilter());
  factory->registerPrototype(new MelFilterBank());
  factory->registerPrototype(new Normalize());
  factory->registerPrototype(new OBSI());
  factory->registerPrototype(new RMS());
  factory->registerPrototype(new Rolloff());
  factory->registerPrototype(new ShapeStatistics());
  factory->registerPrototype(new Slope());
  factory->registerPrototype(new SlopeIntegrator());
  factory->registerPrototype(new Sqr());
  factory->registerPrototype(new SpectralCrestFactorPerBand());
  factory->registerPrototype(new SpectralFlatnessPerBand());
  factory->registerPrototype(new StatisticalIntegrator());
  factory->registerPrototype(new Sum());
  factory->registerPrototype(new Variation());
  factory->registerPrototype(new ZCR());
  factory->registerPrototype(new Join());
  factory->registerPrototype(new AdvancedFrameTokenizer());
  factory->registerPrototype(new CQT());
  factory->registerPrototype(new ChordDictDecoder());
  factory->registerPrototype(new Chroma2ChordDict());
  factory->registerPrototype(new ChromaReduce());
  factory->registerPrototype(new ChromaTune());
  factory->registerPrototype(new Chroma2());
  factory->registerPrototype(new Concatenate());
  factory->registerPrototype(new Decimate2());
  factory->registerPrototype(new DvornikovDifferentiator());
  factory->registerPrototype(new HistogramSummary());
  factory->registerPrototype(new LogCompression());
  factory->registerPrototype(new NormalizeMaxAll());
}
