#!/usr/bin/env python
# -*- coding: ISO-8859-1 -*-
#
# Yaafe
#
# Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
# Télécom ParisTech / dept. TSI
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


import sys
from optparse import OptionParser

try:
    import yaafelib as yaafe
except ImportError, e:
    print 'ERROR: cannot load yaafe packages: ', e
    sys.exit()

if (yaafe.loadComponentLibrary('yaafe-io') != 0):
    print 'WARNING: cannot load yaafe-io component library !'
output_format_choices = ['csv']
if yaafe.isComponentAvailable('H5DatasetWriter'):
    output_format_choices.append('h5')

def listFeatures():
    features = [feat.__name__
                for feat in yaafe.AudioFeatureFactory.get_all_features()
                if not feat.TRANSFORM]
    transforms = [feat.__name__
                  for feat in yaafe.AudioFeatureFactory.get_all_features()
                  if feat.TRANSFORM]
    if len(features) == 0:
        print 'No features available ! Please check that YAAFE_PATH env var is set correctly.'
        return
    features.sort()
    transforms.sort()
    list_features = ['Available features:']
    list_features.extend([' - '+f for f in features])
    list_features.append('Available feature transforms:')
    list_features.extend([' - '+f for f in transforms])
    list_features.append('Available Output formats:')
    list_features.extend([' - '+f for f in yaafe.getOutputFormatList()])

    list_features_str = '\n'.join(list_features)
    print list_features_str
    return list_features_str


def describeFeature(name):
    feat = yaafe.AudioFeatureFactory.get_feature(name)
    print feat.__doc__
    params = feat.get_parameters()
    print 'Parameters are :'
    for (name, defaultValue, desc) in params:
        print '- %s (default=%s): %s' % (name, defaultValue, desc)


def describeOutputFormat(name):
    print ''
    print '[%s] %s' % (name, yaafe.getOutputFormatDescription(name))
    print ''
    print 'Parameters are:'
    for (p, default, desc) in yaafe.getOutputFormatParameters(name):
        print '- %s (default=%s): %s' % (p, default, desc)


def showFeatures(h5file):
    fdict = yaafe.readH5FeatureDescriptions(h5file)
    if len(fdict) == 0:
        print '%s contains no yaafe features' % h5file
        return
    for name, fd in fdict.iteritems():
        print '%s [%ix%i] %iHz:' % (name, fd['dim'], fd['nbframes'],
                                    fd['sampleRate'])
        print '  - blockSize = %i' % fd['blockSize']
        print '  - stepSize = %i' % fd['stepSize']
        for key, val in fd['attrs'].iteritems():
            print '  - %s = "%s"' % (key, val)
        print ''


def main():

    parser = OptionParser(version='yaafe.py, Yaafe v%s' %
                          yaafe.getYaafeVersion())
    parser.add_option('-v', '--verbose', dest='verbose', action='store_true',
                      default=False, help='display more output')
    parser.add_option('-l', '--list', dest='listFeatures', action='store_true',
                      default=False,
                      help='list all available features and output formats')
    parser.add_option('-d', '--describe', dest='describe', default=None,
                      help='describe a feature or an output format')
    parser.add_option('-f', '--feature', action='append', dest='feature',
                      metavar='FEATUREDEFINITION', help='feature to extract')
    parser.add_option('-c', '--config-file', dest='configFile', default=None,
                      help='feature extraction plan')
    parser.add_option('-r', '--rate', dest='sample_rate', type='int',
                      default=None,
                      help='working samplerate in Hz.')
    parser.add_option('', '--resample', dest='resample', action='store_true',
                      default=False,
                      help='Resample input signal to the analysis sample rate')
    parser.add_option('-n', '--normalize', dest='normalize',
                      action='store_true', default=False,
                      help='normalize input signal by removing mean and scale maximum absolute value to 0.98 (or other value given with --normalize-max)')
    parser.add_option('', '--normalize-max', dest='normalize_max',
                      type='float', default=0.98,
                      help='Normalize input signal so that maximum absolute value reached given value (see -n, --normalize)')
    parser.add_option('-i', '--input', dest='input_list', default=None,
                      help='text file, each line is an audio file to process')
    parser.add_option('-b', '--base-dir', dest='out_dir', default='',
                      help='output directory base')
    parser.add_option('-o', '--output-format', dest='format', default='csv',
                      choices=output_format_choices,
                      help='Features output format: %s' % '|'.join(output_format_choices))
    parser.add_option('-p', '--output-params', dest='formatparams',
                      action='append', default=[],
                      metavar='key=value',
                      help='add an output format parameter (can be used multiple times, use -l options to list output formats and parameters)')
    parser.add_option('', '--dump-dataflow', dest='dumpDataflow', default='',
                      metavar='FILE',
                      help='output dataflow plan (suitable for process with yaafe-engine)')
    parser.add_option('', '--dump-graph', dest='dumpGraph', default='',
                      metavar='FILE',
                      help="output dataflow in dot format (suitable for display with graphviz")
    parser.add_option('-s', '--data-block-size', dest='buffer_size',
                      type='int', default=None,
                      help='Prefered size for data blocks.')
    parser.add_option('', '--show', dest='showFeatures', default=None,
                      help='Show all features in a H5 file')

    (options, audiofiles) = parser.parse_args()

    if options.listFeatures:
        listFeatures()
        return
    if options.describe:
        if options.describe in yaafe.getOutputFormatList():
            describeOutputFormat(options.describe)
        else:
            describeFeature(options.describe)
        return
    if options.showFeatures:
        showFeatures(options.showFeatures)
        return
    if not options.sample_rate:
        print "ERROR: please specify sample rate !"
        return
    if options.buffer_size:
        yaafe.setPreferedDataBlockSize(options.buffer_size)
    if options.verbose:
        yaafe.setVerbose(True)

    # initialize feature plan
    fp = yaafe.FeaturePlan(sample_rate=options.sample_rate,
                           normalize=(options.normalize_max
                                      if options.normalize else None),
                           resample=options.resample)

    if options.configFile:
        if not fp.loadFeaturePlan(options.configFile):
            return
    elif options.feature:
        for feat in options.feature:
            if not fp.addFeature(feat):
                return
    else:
        print "ERROR: please specify features using either a config file or -f [feature]"
        return

    if options.dumpDataflow:
        fp.getDataFlow().save(options.dumpDataflow)
    if options.dumpGraph:
        fp.getDataFlow().save(options.dumpGraph)

    # read audio file list
    if options.input_list:
        fin = open(options.input_list, 'r')
        for line in fin:
            audiofiles.append(line.strip())
        fin.close()

    if audiofiles:
        # initialize engine
        engine = yaafe.Engine()
        if not engine.load(fp.getDataFlow()):
            return
        # initialize file processor
        afp = yaafe.AudioFileProcessor()
        oparams = dict()
        for pstr in options.formatparams:
            pstrdata = pstr.split('=')
            if len(pstrdata) != 2:
                print 'ERROR: invalid parameter syntax in "%s" (should be "key=value")' % pstr
                return
            oparams[pstrdata[0]] = pstrdata[1]
        afp.setOutputFormat(options.format, options.out_dir, oparams)
        # process audio files
        for audiofile in audiofiles:
            afp.processFile(engine, audiofile)

if __name__ == '__main__':
    main()
