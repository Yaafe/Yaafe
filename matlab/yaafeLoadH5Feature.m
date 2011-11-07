function data = yaafeLoadH5Feature(filename,featname,frames)
%Load features data from a Yaafe HDF5 file
%
%   DATA = yaafeLoadH5Feature(FILEMNAME, FEATNAME) returns all the data from
%   the HDF5 file FILENAME related to the feature FEATNAME.
%
%   DATA = yaafeLoadH5Feature(FILEMNAME, FEATNAME, N) returns only the
%   first N frames of the data from the HDF5 file FILENAME related to the
%   feature FEATNAME.
%
%   DATA = yaafeLoadH5Feature(FILEMNAME, FEATNAME, [N1 N2])
%   returns only the frames N1 through N2 of the data from the HDF5 file
%   FILENAME related to the feature FEATNAME.
%
%   Example:
%
%     % Let's suppose that the file has been generated using this call :
%     % yaafe -r 44100 -f "MFCC blockSize=1024 stepSize=512" test.wav
%
%     % Read all dataset from a Yaafe hdf5file
% data1 = yaafeLoadH5Feature('test.wav.h5','MFCC');
%
%     % Read 30 first frames of the dataset
% data2 = yaafeLoadH5Feature('test.wav.h5','MFCC',30);
%
%     % Read frames 20 to 40 from the dataset
% data = yaafeLoadH5Feature('test.wav.h5','MFCC', [20 40]);

% display(sprintf('load %s:%s',filename,path));

% Check File existence
if ~exist(filename, 'file'), return; end;

if nargin == 3,
    GET_SUBSET = true;
    if length(frames) == 1,
        offset = 0;
        slabsize = frames;
    else
        offset = frames(1)-1;
        slabsize = frames(2) - frames(1)  +1;
    end
else
    GET_SUBSET = false;
end

%% Open HDF5 File
fileID = H5F.open(filename,'H5F_ACC_RDONLY','H5P_DEFAULT');
datasetID = H5D.open(fileID,featname);

%% Select Data Subset
if GET_SUBSET
    %Get dataspace ID
    dataspaceID = H5D.get_space(datasetID);

    % select the hyperslab
    stride = ones(size(offset));
    count = ones(size(offset));
    H5S.select_hyperslab(dataspaceID, 'H5S_SELECT_SET', offset, stride, count, slabsize);

    % create space for the hyperslab in memory
    memspaceID = H5S.create_simple(length(slabsize), slabsize, slabsize);

else
    dataspaceID = 'H5S_ALL';
    memspaceID = 'H5S_ALL';
end

%% Get Dataset
% Read Data
data = H5D.read(datasetID, 'H5ML_DEFAULT', memspaceID, dataspaceID, 'H5P_DEFAULT');

% Check & Correct datasize if needed
typeID = H5D.get_type(datasetID);
[output ndims offset] = H5T.get_array_dims(typeID);
if (size(data,1) == ndims)
    data = transpose(data);
end

%% Close HDF5 File
if GET_SUBSET
    H5S.close(dataspaceID);
    H5S.close(memspaceID);
end
H5D.close(datasetID);
H5F.close(fileID);
end
