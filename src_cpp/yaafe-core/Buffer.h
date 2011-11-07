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

#ifndef BUFFER_H_
#define BUFFER_H_

#include <list>
#include <assert.h>

namespace YAAFE
{
/**
 * StreamInfo describe stream.
 * - sampleRate is the sampleRate of original signal
 * - sampleStep is the gap between consecutive frames in number of original samples
 * - frameLength is the length of a frame in number of original samples
 * - size is the dimension of stream. A mono signal has a size 1,
 */
class StreamInfo {
public:
	StreamInfo();
	StreamInfo(const StreamInfo& inInfo, int outSize);
	~StreamInfo();
	bool operator==(const StreamInfo& info) const;
	double sampleRate;
	int sampleStep;
	int frameLength;
	int size;
};

inline bool StreamInfo::operator==(const StreamInfo& info) const
{
	return ((size==info.size) &&
			(sampleRate==info.sampleRate) &&
			(sampleStep==info.sampleStep) &&
			(frameLength==info.frameLength));
}

/**
 * DataBlock manage a memory block used to store numerical data streams.
 * It's usually created by an OutputBuffer and filled with data. Then can
 * be read by several InputBuffer. When no references to a block remains
 * it can be reused or freed.
 */
class DataBlock {
public:
	~DataBlock();

	int size; // dimension of a token (each dimension is a double)
	int tokens; // number of tokens in the data block
	int maxtokens; // maximum number of tokens in the allocated memory block
	int numref; // number of reference to this block
	double* data; // pointer to data

	/**
	 * Return how many tokens can be written in the remaining memory.
	 */
	inline int remaining() { return maxtokens-tokens; };

	/**
	 * Return a pointer to the i-th token
	 */
	inline double* operator[](size_t i) { return data + size * i; };

	/**
	 * Return a const pointer to the i-th token
	 */
	inline const double* operator[](size_t i) const { return data + size * i; };

	/**
	 * Create a data block with the given token size.
	 */
	static DataBlock* create(int size);

	/**
	 * acquire a reference to a data block
	 */
	static void acquire(DataBlock* db);

	/**
	 * release a reference to a data block
	 */
	static void release(DataBlock* db);

	/**
	 * Get the prefered block size. If possible, the create method will create
	 * blocks of this size (total number of doubles)
	 */
	static int preferedBlockSize();

	/**
	 * Set the prefered block size.
	 */
	static void setPreferedBlockSize(int size);

private:
	DataBlock();
	DataBlock(const DataBlock& db);
	DataBlock& operator=(const DataBlock& db);

	static int s_preferedBlockSize;
};

/**
 * InputBuffer represents a input data stream where a component
 * can read data.
 *
 * Here's some classical way to deal with InputBuffer
 *
 * Process token one by one
 *
 * InputBuffer* in = ....; // in is an InputBuffer*
 * while (!in->empty()) {
 *   double* data = in->readToken();
 *   // do something with the token
 *   in->consumeToken(); // token is no more valid
 * }
 *
 * Process token by 3
 *
 * InputBuffer* in = ....; // in is an InputBuffer*
 * while (in->hasTokens(3)) {
 *   double* tok0 = in->token(0);
 *   double* tok1 = in->token(1);
 *   double* tok2 = in->token(2);
 *   // do something with tok0, tok1 and tok2
 *   in->consumeTokens(3); // token are no more valid
 * }
 *
 * Process block of tokens:
 *
 * InputBuffer* in = ....; // in is an InputBuffer*
 * while (!in->empty()) {
 *   int nbtokens = blockAvailableTokens();
 *   double* data = in->readToken();
 *   // data points to a block of nbtokens contiguous in memory
 *   // do something with data
 *   in->consumeBlock(); // consume the nbtokens tokens
 * }
 */
class InputBuffer {
	friend class OutputBuffer;
public:
	InputBuffer(const StreamInfo& info);
	~InputBuffer();

	/**
	 * Returns the data stream info
	 */
	const StreamInfo& info() const { return _info; };

	/**
	 * Get the current token number, first token is 0
	 */
	int tokenno() const { return _tokenno + _pos; };

	/**
	 * Get the token size (number of double)
	 */
	int size() const { return _info.size; }

	/**
	 * Returns true if no token is available
	 */
	bool empty() { return (((_data==NULL) || (_data->tokens==_pos)) && _queue.empty()); }

	/**
	 * Return a pointer to the current token. The pointer is valid
	 * as long as the current has not been consumed.
	 */
	double* readToken() { return (*_data)[_pos]; };

	/**
	 * Consume the current token.
	 */
	void consumeToken() { if (++_pos==_data->tokens) blockConsume(); }

	/**
	 * Check if toks tokens are availble
	 */
	bool hasTokens(int toks);

	/**
	 * Returns the number of tokens available to read
	 */
	int availableTokens();

	/**
	 * Returns a pointer to the i-th token.
	 */
	inline double* token(int i);

	/**
	 * Read toks tokens into a buffer. The buffer must have enough memory allocated
	 * to store size*toks doubles.
	 */
	int read(double* buf,int toks);

	/**
	 * Consume toks tokens
	 */
	void consumeTokens(int toks);

	/**
	 * Returns how many tokens are available in a contiguous memory block
	 */
	int blockAvailableTokens() { if (_data==NULL) blockConsume(); return (_data==NULL) ? 0 : _data->tokens - _pos; };

	/**
	 * Returns a pointer to the i-th tokens assuming that it is in the current contiguous
	 * memory block
	 */
	double* blockToken(int i) { return (*_data)[_pos+i]; }

	/**
	 * Consume the current memory block
	 */
	void blockConsume();

	/**
	 * Prepend toks tokens of zeros before the first token
	 */
	void prependZeros(int toks);

	/**
	 * Writte toks tokens of zeros.
	 */
	void appendZeros(int toks);

	/**
	 * Release all data blocks
	 */
	void clear();

	// just for debugging
	void debug();

private:
	// prevent copy
	InputBuffer(const InputBuffer& in) {};

	StreamInfo _info;
	DataBlock* _data;
	int _tokenno;
	int _pos;
	std::list<DataBlock*> _queue;

};

/**
 * OutputBuffer represents an output data stream where a component
 * can write data.
 */
class OutputBuffer {
public:
	OutputBuffer(const StreamInfo& info);
	~OutputBuffer();

/**
 * Methods used by components
 */

	int size() const { return _info.size; } // token size
	const StreamInfo& info() const { return _info; } // data stream info

	/**
	 * returns how many tokens can be written in the remaining
	 * contiguous memory block
	 */
	int remainingSpace() const { return _data->remaining(); };

	/**
	 * Get a pointer where to write a token
	 */
	inline double* writeToken();

	/**
	 * Write the toks tokens from buffer buf
	 */
	int write(double* buf,int toks);

	/**
	 * Return the current token number
	 */
	int tokenno() const { return _tokenno; };


/**
 * Methods used by engine
 */

	// bind to an input buffer
	void bindInputBuffer(InputBuffer* r);

	// dispatch all complete data block to bound input buffers
	void dispatch();

	// consided last block as a complete block
	void flush();

	// release all data blocks
	void clear();

	// just for debugging
	void debug();

private:
	void nextBlock();

	StreamInfo _info;
	DataBlock* _data;
	std::list<DataBlock*> _queue;
	std::list<InputBuffer*> _readers;
	int _tokenno;
};

double* OutputBuffer::writeToken() {
	_tokenno++;
	double* d = (*_data)[_data->tokens++];
	if (_data->remaining()==0) nextBlock();
	return d;
}

inline OutputBuffer* buildOutputBufferFromInfo(const StreamInfo& info) {
	return new OutputBuffer(info);
}

inline double* InputBuffer::token(int i) {
	if (i<(_data->tokens-_pos))
		return (*_data)[_pos+i];
	i -= _data->tokens-_pos;
	for (std::list<DataBlock*>::const_iterator it=_queue.begin();
			it!=_queue.end();it++)
	{
		if (i<(*it)->tokens)
			return (**it)[i];
		i -= (*it)->tokens;
	}
	return NULL;
}

}


#endif /* BUFFER_H_ */
