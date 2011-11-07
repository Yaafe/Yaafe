function attrs = yaafeLoadH5FeatureAttributes(filename,featname)
%Load features attributes from a Yaafe HDF5 file
%
%   ATTRS = yaafeLoadH5FeatureAttributes(FILEMNAME, FEATNAME)
% returns the attribute from the HDF5 file FILENAME related to the feature
% FEATNAME
%
%   Example:
%
%     % Read attributes from a Yaafe hdf5file
%     % Let's suppose that the file has been generated using this call :
%     % yaafe -r 44100 -f "MFCC blockSize=1024 stepSize=512" test.wav
%  attrs = yaafeLoadH5Feature('test.wav.h5','MFCC');

% display(sprintf('load %s:%s',filename,path));

% Check File existence
if ~exist(filename, 'file'), return; end;

%% Open HDF5 File
fileID = H5F.open(filename,'H5F_ACC_RDONLY','H5P_DEFAULT');
datasetID = H5D.open(fileID,featname);

%% Get Attributes
attrs = struct;
numAttrs = H5A.get_num_attrs(datasetID);
for i = 0:numAttrs-1
    attrID = H5A.open_idx(datasetID,i);
    key = H5A.get_name(attrID);
    value = H5A.read(attrID,'H5ML_DEFAULT');
    if isnumeric(value)
        attrs.(key) = double(value);
    elseif ischar(value)
        attrs.(key) = value(1:end-1)';
    else
        warning('Value format not supported in yaafeLoadH5FeatureAttributes');
    end
    H5A.close(attrID);
end

% Add the feature dimension has an attribute
typeID = H5D.get_type(datasetID);
[output ndims offset] = H5T.get_array_dims(typeID);
attrs.featDim = ndims;

% Add the number of frames as an attribute
%Get dataspace ID
dataspaceID = H5D.get_space(datasetID);
%Get the number of element in the current dataspace selection
attrs.nbFrames = H5S.get_select_npoints(dataspaceID);

%% Close HDF5 File
H5S.close(dataspaceID);
H5D.close(datasetID);
H5F.close(fileID);
end
