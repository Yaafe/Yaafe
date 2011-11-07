% Use Yaafe class to extract features with Yaafe.
%
% The Yaafe class needs a Yaafe Dataflow file to be configured. You can get
% a Yaafe Dataflow file with the --dump-dataflow option of the yaafe.py 
% script. For more details, please visit 
% http://yaafe.sourceforge.net/manual/tools.html#matlab-wrapper
%
% Example of usage of Yaafe class to extract features over a signal:
%
%    >> yaafe = Yaafe();
%    >> yaafe.load('yaflow')
%    1 
%
% 'yaflow' is the Dataflow file. the 'load' method returns 1 if success or
% 0 if failed. 
%
%    >> signal = rand(1,100000);
%    >> feats1 = yaafe.process(signal);
%    >> feats2 = yaafe.processFile('song.wav');
%    >> feats1
%
%    feats1 = 
%
%          mfcc: [1x1 struct]
%       mfcc_d1: [1x1 struct]
%       mfcc_d2: [1x1 struct]
%            sf: [1x1 struct]
%            sr: [1x1 struct]
%
% Once Dataflow file is loaded, you can call the 'process' and
% 'processFile' methods as many times as you want. The output is a struct
% where each fields holds an audio features with the following metadata:
%
%           name: the feature name has defined in the Dataflow file
%           size: size of the feature
%     sampleRate: analysis sample rate
%     sampleStep: number of sample between consecutive analysis windows
%    frameLength: length of analysis window
%           data: feature values
%
classdef Yaafe < handle

    properties(Access = private)
        engine
    end
    
    methods(Access = public)
        
        function self = Yaafe()
            self.engine = -1;
        end
        
        function delete(self)
            if (self.engine>=0)
                yaafemex('destroy',self.engine);
            end
        end
        
        function ok = load(self, filename)
            if (self.engine>=0)
                yaafemex('destroy',self.engine);
            end
            self.engine = yaafemex('load',filename);
            ok = (self.engine >= 0);
        end
        
        function print(self)
            yaafemex('print',self.engine);
        end
        
        function feats = process(self, signal)
            feats = yaafemex('process',self.engine,signal);
        end
        
        function feats = processFile(self, filename)
            feats = yaafemex('processfile',self.engine,filename);
        end
    end
end

