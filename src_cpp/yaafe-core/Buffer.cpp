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

#include "Buffer.h"
#include "utils.h"
#include <assert.h>
#include <vector>
#include <iostream>
#include <string.h>

#define PREFERRED_DATABLOCK_SIZE 8192

using namespace std;

namespace YAAFE {

StreamInfo::StreamInfo() :
	sampleRate(0), sampleStep(0), frameLength(0), size(0) {
}

StreamInfo::StreamInfo(const StreamInfo& inInfo, int outSize) :
	sampleRate(inInfo.sampleRate), sampleStep(inInfo.sampleStep), frameLength(inInfo.frameLength), size(outSize)
{}

StreamInfo::~StreamInfo() {
}

DataBlock::DataBlock()
{
}

DataBlock::DataBlock(const DataBlock& db)
{
	// never called
}

DataBlock::~DataBlock()
{
	delete [] data;
}

DataBlock& DataBlock::operator=(const DataBlock& db)
{
	// never called
}

int DataBlock::s_preferedBlockSize(PREFERRED_DATABLOCK_SIZE);

int DataBlock::preferedBlockSize() {
	return s_preferedBlockSize;
}

void DataBlock::setPreferedBlockSize(int size) {
	s_preferedBlockSize = size;
}

DataBlock* DataBlock::create(int size) {
	DataBlock* db = new DataBlock();
	db->size = size;
	db->tokens = 0;
	db->numref = 1;
	db->maxtokens = (size>s_preferedBlockSize ? 1 : s_preferedBlockSize / size);
	db->data = new double[db->size*db->maxtokens];
	return db;
}

void DataBlock::acquire(DataBlock* db) {
	db->numref++;
}

void DataBlock::release(DataBlock* db) {
	if (db==NULL)
		return;
//	cerr << "release block " << db << " : numref -> " << db->numref-1 << endl;
	if (--(db->numref) == 0) {
		delete db;
	}
}


InputBuffer::InputBuffer(const StreamInfo& info) :
		_info(info), _data(), _pos(0), _tokenno(0), _queue()
{}

InputBuffer::~InputBuffer()
{
	clear();
}

bool InputBuffer::hasTokens(int toks) {
	if (_data==NULL) {
		blockConsume();
		if (_data==NULL) return false;
	}
	int av = _data->tokens - _pos;
	if (av>=toks) return true;
	for (list<DataBlock*>::const_iterator it=_queue.begin(); it!=_queue.end(); it++)
	{
		av += (*it)->tokens;
		if (av>=toks) {
			return true;
		}
	}
	return false;
}

int InputBuffer::availableTokens() {
	if (_data==NULL)
		blockConsume();
	int av = (_data ? _data->tokens - _pos : 0);
	for (list<DataBlock*>::const_iterator it=_queue.begin(); it!=_queue.end(); it++)
		av += (*it)->tokens;
	return av;
}

int InputBuffer::read(double* buf,int toks) {
	if (_data==NULL) {
		blockConsume();
		if (_data==NULL)
			return 0;
	}
	int read = min(toks,_data->tokens -_pos);
	const int tokSize = _data->size;
	memcpy(buf,(*_data)[_pos],read*tokSize*sizeof(double));
	if (read==toks) return read;
	for (list<DataBlock*>::const_iterator it=_queue.begin(); it!=_queue.end(); it++)
	{
		int toRead = min(toks-read,(*it)->tokens);
		memcpy(buf + read*tokSize ,(*it)->data,toRead*tokSize*sizeof(double));
		read += toRead;
		if (read==toks) break;
	}
	return read;
}

void InputBuffer::clear() {
	for (list<DataBlock*>::iterator it=_queue.begin();it!=_queue.end();it++)
		DataBlock::release(*it);
	_queue.clear();
	DataBlock::release(_data);
	_data = NULL;
	_pos = 0;
	_tokenno = 0;
}

void InputBuffer::consumeTokens(int toks) {
	if (_data==NULL) {
		blockConsume();
		if (_data==NULL)
			return;
	}
	int f=min(toks,_data->tokens-_pos);
	_pos += f;
	if (_pos == _data->tokens) {
		blockConsume();
	}
	while (_data && f<toks) {
		int toForward = min(toks-f,_data->tokens);
		_pos += toForward;
		f += toForward;
		if (_pos == _data->tokens) blockConsume();
	}
}

void InputBuffer::blockConsume() {
//	cerr << "InputBuffer::nextBlock()" << endl;
	if (_data!=NULL) {
		_tokenno += _data->tokens;
		DataBlock::release(_data);
	}
	_data = NULL;
	_pos = 0;
	if (!_queue.empty()) {
		_data = _queue.front();
		_queue.pop_front();
	}
}

void InputBuffer::prependZeros(int toks) {
	assert(_pos==0); // cannot prepend zeros if buffer has already been read
	int written = 0;
	while (written<toks) {
		if (_data!=NULL) {
			_queue.push_front(_data);
		}
		_data = DataBlock::create(_info.size);
		int toWrite = min(toks-written,_data->maxtokens);
		double* d=_data->data;
		for (int i=0;i<(toWrite * _info.size);i++)
			d[i] = 0.0;
		_data->tokens=toWrite;
		written+=toWrite;
	}
	_tokenno -= toks;
}

void InputBuffer::appendZeros(int toks) {
	int written = 0;
	while (written<toks) {
		DataBlock* db = DataBlock::create(_info.size);
		int toWrite = min(toks-written,db->maxtokens);
		double* d=db->data;
		for (int i=0;i<(toWrite * _info.size);i++)
			d[i] = 0.0;
		db->tokens=toWrite;
		written+=toWrite;
		_queue.push_back(db);
	}
}


void InputBuffer::debug() {
	cerr << "InputBuffer " << this << ": ";
	if (_data!=NULL) {
		cerr << "head: " << _data << " "<< _pos << "/" << _data->tokens << "[" << _data->maxtokens << "]";
	}
	for (list<DataBlock*>::const_iterator it=_queue.begin();it!=_queue.end();it++)
		cerr << " | " << *it << " " << (*it)->tokens << " [" << (*it)->maxtokens << "]";
	cerr << endl;
}

void OutputBuffer::debug() {
	cerr << "OutputBuffer " << this << ": ";
	if (_data!=NULL) {
		cerr << "head: " << _data << " "<< _data->tokens << "[" << _data->maxtokens << "]";
	}
	for (list<DataBlock*>::const_iterator it=_queue.begin();it!=_queue.end();it++)
		cerr << " | " << *it << " " << (*it)->tokens << " [" << (*it)->maxtokens << "]";
	cerr << endl;
}

OutputBuffer::OutputBuffer(const StreamInfo& info) :
		_info(info), _queue(), _tokenno(0)
{
	_data = DataBlock::create(_info.size);
}

OutputBuffer::~OutputBuffer()
{
	clear();
	DataBlock::release(_data);
	_data = NULL;
}

void OutputBuffer::nextBlock() {
//	cerr << "OutputBuffer::nextBlock()" << endl;
	_queue.push_back(_data);
	_data = DataBlock::create(_info.size);
}

int OutputBuffer::write(double* buf,int toks) {
	int written = 0;
	while (written<toks) {
		int toWrite = min(_data->maxtokens-_data->tokens,toks-written);
		memcpy((*_data)[_data->tokens],buf + _data->size*written,toWrite*_data->size*sizeof(double));
		written += toWrite;
		_data->tokens += toWrite;
		if (_data->tokens == _data->maxtokens)
			nextBlock();
	}
	_tokenno += written;
	return written;
}

void OutputBuffer::bindInputBuffer(InputBuffer* r) {
	_readers.push_back(r);
}

void OutputBuffer::dispatch() {
//	cerr << "OutputBuffer dispatches " << _queue.size() << " blocks"  << endl;
	if (_queue.empty())
		return;
	for (list<InputBuffer*>::iterator it=_readers.begin();
			it!=_readers.end(); it++)
	{
		list<DataBlock*>& q = (*it)->_queue;
		for (list<DataBlock*>::iterator dbit=_queue.begin();dbit!=_queue.end();dbit++) {
			DataBlock::acquire(*dbit);
			q.push_back(*dbit);
		}
		if ((*it)->_data==NULL)
		{
			(*it)->_data = q.front();
			q.pop_front();
		}
	}
	for (list<DataBlock*>::iterator dbit=_queue.begin();dbit!=_queue.end();dbit++)
		DataBlock::release(*dbit);
	_queue.clear();
}

void OutputBuffer::flush() {
	if (_data->tokens>0)
		nextBlock();
	dispatch();
}

void OutputBuffer::clear() {
	for (list<DataBlock*>::iterator dbit=_queue.begin();dbit!=_queue.end();dbit++)
		DataBlock::release(*dbit);
	_queue.clear();
	_data->tokens = 0;
	_tokenno = 0;
}

}
