#include <array>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <endian.h>
#ifdef _MSC_VER
#include <inttypes.h>
#include <intrin.h>
#endif
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;

uint64_t my_rand() // some stupid random function for proof of concept.
{
	static uint64_t a = 4;
	static uint64_t state = 0xF174372375725326;
	state = ~(a * state + 90877);
	a = (state ^ a) + 0x1111111111111111;
	a = (a << 5) ^ (state >> 40);
	a ^= state;
	state = (state >> 3) | (state << 61);
	return a;
}

class ZAObject
{
public:
	virtual void toIdentity() = 0;
	virtual void toZero() = 0;
	virtual void randomize() = 0;
	virtual void read(const BYTE *buf) = 0;
	virtual void write(BYTE *buf) const = 0;
	virtual void print() const = 0;
};

template <typename T, typename TL, DWORD P>
class ZAIntegerModP : public ZAObject
{
public:
	static_assert(sizeof(TL) >= 2 * sizeof(T), "TL must be at least twice in size than T");
	inline static size_t size()
	{
		return sizeof(T);
	}

private:
	T _val;

public:
	ZAIntegerModP() : _val(0) {}

	virtual void toIdentity()
	{
		_val = 1;
	}
	virtual void toZero()
	{
		_val = 0;
	}
	virtual void randomize()
	{
		_val = my_rand() % P;
	}
	virtual void read(const BYTE *buf)
	{
		if (sizeof(T) == 1)
			_val = buf[0];
		if (sizeof(T) == 2)
			_val = le16toh(*(uint16_t *)buf) % P;
		if (sizeof(T) == 4)
			_val = le32toh(*(uint32_t *)buf) % P;
		if (sizeof(T) == 8)
			_val = le64toh(*(uint64_t *)buf) % P;
	}
	virtual void write(BYTE *buf) const
	{
		if (sizeof(T) == 1)
			buf[0] = _val;
		if (sizeof(T) == 2)
			*(uint16_t *)buf = htole16(_val);
		if (sizeof(T) == 4)
			*(uint32_t *)buf = htole32(_val);
		if (sizeof(T) == 8)
			*(uint64_t *)buf = htole64(_val);
	}
	virtual void print() const
	{
		std::cout << (uint64_t)_val;
	}

	friend inline const ZAIntegerModP<T, TL, P> operator*(const ZAIntegerModP<T, TL, P> &l, const ZAIntegerModP<T, TL, P> &r)
	{
		ZAIntegerModP<T, TL, P> m;
		m._val = ((TL)l._val * (TL)r._val) % P;
		return m;
	}
	friend inline const ZAIntegerModP<T, TL, P> operator+(const ZAIntegerModP<T, TL, P> &l, const ZAIntegerModP<T, TL, P> &r)
	{
		ZAIntegerModP<T, TL, P> m;
		m._val = ((TL)l._val + (TL)r._val) % P;
		return m;
	}
	friend inline const ZAIntegerModP<T, TL, P> operator-(const ZAIntegerModP<T, TL, P> &l, const ZAIntegerModP<T, TL, P> &r)
	{
		ZAIntegerModP<T, TL, P> m;
		m._val = (P + (TL)l._val - (TL)r._val) % P;
		return m;
	}
	ZAIntegerModP<T, TL, P> &operator+=(const ZAIntegerModP<T, TL, P> &r)
	{
		_val = ((TL)_val + (TL)r._val) % P;
		return *this;
	}
};

template <typename T>
class ZAInteger : public ZAObject
{
public:
	inline static size_t size()
	{
		return sizeof(T);
	}

private:
	T _val;

public:
	ZAInteger() : _val(0) {}

	virtual void toIdentity()
	{
		_val = 1;
	}
	virtual void toZero()
	{
		_val = 0;
	}
	virtual void randomize()
	{
		_val = my_rand();
	}
	virtual void read(const BYTE *buf)
	{
		if (sizeof(T) == 1)
			_val = buf[0];
		if (sizeof(T) == 2)
			_val = le16toh(*(uint16_t *)buf);
		if (sizeof(T) == 4)
			_val = le32toh(*(uint32_t *)buf);
		if (sizeof(T) == 8)
			_val = le64toh(*(uint64_t *)buf);
	}
	virtual void write(BYTE *buf) const
	{
		if (sizeof(T) == 1)
			buf[0] = _val;
		if (sizeof(T) == 2)
			*(uint16_t *)buf = htole16(_val);
		if (sizeof(T) == 4)
			*(uint32_t *)buf = htole32(_val);
		if (sizeof(T) == 8)
			*(uint64_t *)buf = htole64(_val);
	}
	virtual void print() const
	{
		std::cout << (uint64_t)_val;
	}

	friend inline const ZAInteger<T> operator*(const ZAInteger<T> &l, const ZAInteger<T> &r)
	{
		ZAInteger<T> m;
		m._val = l._val * r._val;
		return m;
	}
	friend inline const ZAInteger<T> operator+(const ZAInteger<T> &l, const ZAInteger<T> &r)
	{
		ZAInteger<T> m;
		m._val = l._val + r._val;
		return m;
	}
	friend inline const ZAInteger<T> operator-(const ZAInteger<T> &l, const ZAInteger<T> &r)
	{
		ZAInteger<T> m;
		m._val = l._val - r._val;
		return m;
	}
	ZAInteger<T> &operator+=(const ZAInteger<T> &r)
	{

		_val += r._val;
		return *this;
	}
};

template <class T>
class ZAComplex : public ZAObject
{
public:
	inline static size_t size()
	{
		return 2 * T::size();
	}

private:
	T _real, _img;

public:
	virtual void toIdentity()
	{
		_real.toIdentity();
		_img.toZero();
	}
	virtual void toZero()
	{
		_real.toZero();
		_img.toZero();
	}
	virtual void randomize()
	{
		_real.randomize();
		_img.randomize();
	}
	virtual void read(const BYTE *buf)
	{
		_real.read(buf);
		buf += T::size();
		_img.read(buf);
	}
	virtual void write(BYTE *buf) const
	{
		_real.write(buf);
		buf += T::size();
		_img.write(buf);
	}
	virtual void print() const
	{
		std::cout << "<r:";
		_real.print();
		std::cout << ",i:";
		_img.print();
		std::cout << ">";
	}
	friend inline const ZAComplex<T> operator*(const ZAComplex<T> &l, const ZAComplex<T> &r)
	{
		ZAComplex<T> m;
		m._real = l._real * r._real - l._img * r._img;
		m._img = l._real * r._img + l._img * r._real;
		return m;
	}
	friend inline const ZAComplex<T> operator+(const ZAComplex<T> &l, const ZAComplex<T> &r)
	{
		ZAComplex<T> m;
		m._real = l._real + r._real;
		m._img = l._img + r._img;
		return m;
	}
	ZAComplex<T> &operator+=(const ZAComplex<T> &r)
	{
		_real += r._real;
		_img += r._img;
		return *this;
	}
};

template <class T, size_t... Dims>
class ZAMultiDimTable : public ZAObject
{
public:
	static constexpr size_t nDims = sizeof...(Dims);
	static constexpr size_t nElements = (... * Dims);
	static constexpr std::array<size_t, nDims> _indices = {Dims...};

private:
	static constexpr size_t addIndices(size_t ind1, size_t ind2)
	{
		size_t ret = 0;
		size_t mul = 1;
		for (size_t i = 0; i < nDims; ++i)
		{
			ret += mul * ((ind1 + ind2) % _indices[i]);
			ind1 /= _indices[i];
			ind2 /= _indices[i];
			mul *= _indices[i];
		}
		return ret;
	}
	using Elem2d = std::array<std::array<size_t, nElements>, nElements>;
	static constexpr Elem2d initializeIndicesConvert()
	{
		Elem2d conv_map = Elem2d();
		for (size_t i = 0; i < nElements; ++i)
		{
			for (size_t j = 0; j < nElements; ++j)
			{
				conv_map[i][j] = addIndices(i, j);
			}
		}
		return conv_map;
	}

public:
	static constexpr Elem2d conv_map{initializeIndicesConvert()};
	inline static size_t size()
	{
		return nElements * T::size();
	}

private:
	std::array<T, nElements> _table;

public:
	virtual void toIdentity()
	{
		_table[0].toIdentity();
		for (size_t i = 1; i < nElements; ++i)
		{
			_table[i].toZero();
		}
	}
	virtual void toZero()
	{
		for (size_t i = 0; i < nElements; ++i)
		{
			_table[i].toZero();
		}
	}
	virtual void randomize()
	{
		for (size_t i = 0; i < nElements; ++i)
		{
			_table[i].randomize();
		}
	}
	virtual void read(const BYTE *buf)
	{
		for (size_t i = 0; i < nElements; ++i)
		{
			_table[i].read(buf);
			buf += T::size();
		}
	}
	virtual void write(BYTE *buf) const
	{
		for (size_t i = 0; i < nElements; ++i)
		{
			_table[i].write(buf);
			buf += T::size();
		}
	}
	virtual void print() const
	{
		std::cout << "MultiDim"
				  << "{";
		for (size_t i = 0; i < nElements; ++i)
		{
			if (i > 0)
				std::cout << ", ";
			_table[i].print();
		}
		std::cout << "}";
	}

	friend inline const ZAMultiDimTable<T, Dims...> operator*(const ZAMultiDimTable<T, Dims...> &l, const ZAMultiDimTable<T, Dims...> &r)
	{
		ZAMultiDimTable<T, Dims...> m;
		for (size_t i = 0; i < nElements; ++i)
		{
			for (size_t j = 0; j < nElements; ++j)
			{
				m._table[conv_map[i][j]] += l._table[i] * r._table[j];
				//m._table[addIndices(i,j)]+=l._table[i]*r._table[j];
			}
		}
		return m;
	}
	friend inline const ZAMultiDimTable<T, Dims...> operator+(const ZAMultiDimTable<T, Dims...> &l, const ZAMultiDimTable<T, Dims...> &r)
	{
		ZAMultiDimTable<T, Dims...> m;
		for (size_t i = 0; i < nElements; ++i)
		{
			m._table[i] = l._table[i] + r._table[i];
		}
		return m;
	}
	ZAMultiDimTable<T, Dims...> &operator+=(const ZAMultiDimTable<T, Dims...> &r)
	{
		for (size_t i = 0; i < nElements; ++i)
		{
			_table[i] += r._table[i];
		}
		return *this;
	}
};

template <class T, size_t S>
class ZASqMatrix : public ZAObject
{
public:
	inline static size_t size()
	{
		return S * S * T::size();
	}

private:
	std::array<std::array<T, S>, S> _table;

public:
	virtual void toIdentity()
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				if (i == j)
					_table[i][j].toIdentity();
				else
					_table[i][j].toZero();
			}
		}
	}
	virtual void toZero()
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				_table[i][j].toZero();
			}
		}
	}
	virtual void randomize()
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				_table[i][j].randomize();
			}
		}
	}
	virtual void read(const BYTE *buf)
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				_table[i][j].read(buf);
				buf += T::size();
			}
		}
	}
	virtual void write(BYTE *buf) const
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				_table[i][j].write(buf);
				buf += T::size();
			}
		}
	}
	virtual void print() const
	{
		std::cout << "Mat"
				  << "[";
		for (size_t i = 0; i < S; ++i)
		{
			if (i > 0)
				std::cout << ";" << std::endl;
			for (size_t j = 0; j < S; ++j)
			{
				if (j > 0)
					std::cout << ", ";
				_table[i][j].print();
			}
		}
		std::cout << "]" << std::endl;
	}

	friend inline const ZASqMatrix<T, S> operator*(const ZASqMatrix<T, S> &l, const ZASqMatrix<T, S> &r)
	{
		ZASqMatrix<T, S> m;
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					m._table[i][j] += l._table[i][k] * r._table[k][j];
				}
			}
		}
		return m;
	}
	friend inline const ZASqMatrix<T, S> operator+(const ZASqMatrix<T, S> &l, const ZASqMatrix<T, S> &r)
	{
		ZASqMatrix<T, S> m;
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				m._table[i][j] = l._table[i][j] + r._table[i][j];
			}
		}
		return m;
	}
	ZASqMatrix<T, S> &operator+=(const ZASqMatrix<T, S> &r)
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				_table[i][j] += r._table[i][j];
			}
		}
		return *this;
	}
};

template <class T, size_t S>
class ZACubeMatrix
{
public:
	inline static size_t size()
	{
		return S * S * S * T::size();
	}

private:
	std::array<std::array<std::array<T, S>, S>, S> _cube;

public:
	virtual void toIdentity()
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					if (i == j && j == k)
						_cube[i][j][k].toIdentity();
					else
						_cube[i][j][k].toZero();
				}
			}
		}
	}
	virtual void toZero()
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					_cube[i][j][k].toZero();
				}
			}
		}
	}
	virtual void randomize()
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					_cube[i][j][k].randomize();
				}
			}
		}
	}
	virtual void read(const BYTE *buf)
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					_cube[i][j][k].read(buf);
					buf += T::size();
				}
			}
		}
	}
	virtual void write(BYTE *buf) const
	{
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					_cube[i][j][k].write(buf);
					buf += T::size();
				}
			}
		}
	}
	friend inline ZACubeMatrix<T, S> operator*(const ZACubeMatrix<T, S> &l, const ZACubeMatrix<T, S> &r)
	{
		ZACubeMatrix<T, S> m;
		// for (size_t i = 0; i < S; ++i)
		// {
		// 	for (size_t j = 0; j < S; ++j)
		// 	{
		// 		for (size_t k = 0; k < S; ++k)
		// 		{
		// 			for (size_t p = 0; p < S; ++p)
		// 			{
		// 				for (size_t q = 0; q < S; ++q)
		// 				{
		// 					m._cube[i][j][k] +=
		// 						l._cube[i][p][q] * r._cube[q][j][p] +
		// 						l._cube[q][j][p] * r._cube[p][q][k] +
		// 						l._cube[p][q][k] * r._cube[i][p][q];
		// 				}
		// 			}
		// 		}
		// 	}
		// }
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					for (size_t p = 0; p < S; ++p)
					{

						m._cube[i][j][k] +=
							(l._cube[i][j][p] * r._cube[p][j][k] +
							 l._cube[i][p][k] * r._cube[p][j][k] +
							 l._cube[i][j][p] * r._cube[i][p][k]);
					}
				}
			}
		}
		return m;
	}
	friend inline ZACubeMatrix<T, S> operator+(const ZACubeMatrix<T, S> &l, const ZACubeMatrix<T, S> &r)
	{
		ZACubeMatrix<T, S> m;
		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					m._cube[i][j][k] = l._cube[i][j][k] + r._cube[i][j][k];
				}
			}
		}
		return m;
	}
	ZACubeMatrix<T, S> &operator+=(const ZACubeMatrix<T, S> &r)
	{

		for (size_t i = 0; i < S; ++i)
		{
			for (size_t j = 0; j < S; ++j)
			{
				for (size_t k = 0; k < S; ++k)
				{
					_cube[i][j][k] += r._cube[i][j][k];
				}
			}
		}
		return *this;
	}
};

template <size_t BITS>
class ZAKey : public ZAObject
{
public:
	static_assert(BITS % 64 == 0, "BITS must be multiples of 64");
	static_assert(BITS >= 64, "BITS too small");
	static_assert(BITS <= 8192, "BITS too large");
	inline static size_t size()
	{
		return BITS / 8;
	}
	static const size_t n_qw = BITS / 64; // number of quad-words
private:
	std::array<uint64_t, n_qw> _data = {0};

public:
	//	ZAKey() : _data(0) {}

	virtual void toIdentity()
	{
		_data[0] = 1;
		for (size_t i = 1; i < n_qw; ++i)
		{
			_data[i] = 0;
		}
	}
	virtual void toZero()
	{
		for (size_t i = 0; i < n_qw; ++i)
		{
			_data[i] = 0;
		}
	}
	virtual void randomize()
	{
		for (size_t i = 0; i < n_qw; ++i)
		{
			_data[i] = my_rand();
		}
	}
	virtual void read(const BYTE *buf)
	{
		for (size_t i = 0; i < n_qw; ++i)
		{
			_data[i] = le64toh(*(uint64_t *)buf);
			buf += sizeof(uint64_t);
		}
	}
	virtual void write(BYTE *buf) const
	{
		for (size_t i = 0; i < n_qw; ++i)
		{
			*(uint64_t *)buf = le64toh(_data[i]);
			buf += sizeof(uint64_t);
		}
	}
	virtual void print() const
	{
		std::cout << "0x" << std::hex;
		for (ssize_t i = n_qw - 1; i >= 0; --i)
		{
			std::cout << std::setw(16) << std::setfill('0') << _data[i];
		}
		std::cout << std::dec << std::setw(1) << std::endl;
	}
	inline uint64_t operator[](size_t i) const
	{
		return _data[i];
	}
	inline uint64_t &operator[](size_t i)
	{
		return _data[i];
	}
	friend inline const ZAKey<BITS> operator+(const ZAKey<BITS> &l, const ZAKey<BITS> &r)
	{
		ZAKey<BITS> m;
		uint64_t carryin = 0, carryout;
#ifdef _MSC_VER
		for (size_t i = 0; i < n_qw; ++i)
		{
			carryout = _addcarry_u64(carryin, l._data[i], r._data[i], &m._data[i]);
			carryin = carryout;
		}
#else
		for (size_t i = 0; i < n_qw; ++i)
		{
			m._data[i] = __builtin_addcll(l._data[i], r._data[i], carryin, (unsigned long long *)&carryout);
			carryin = carryout;
		}
#endif
		return m;
	}
	friend inline const ZAKey<BITS> operator-(const ZAKey<BITS> &l, const ZAKey<BITS> &r)
	{
		ZAKey<BITS> m;
		uint64_t carryin = 0, carryout;
#ifdef _MSC_VER
		for (size_t i = 0; i < n_qw; ++i)
		{
			carryout = _subborrow_u64(carryin, l._data[i], r._data[i], &m._data[i]);
			carryin = carryout;
		}
#else
		for (size_t i = 0; i < n_qw; ++i)
		{
			m._data[i] = __builtin_subcll(l._data[i], r._data[i], carryin, (unsigned long long *)&carryout);
			carryin = carryout;
		}
#endif
		return m;
	}
	ZAKey<BITS> &operator+=(const ZAKey<BITS> &r)
	{
		uint64_t carryin = 0, carryout;
#ifdef _MSC_VER
		for (size_t i = 0; i < n_qw; ++i)
		{
			carryout = _addcarry_u64(carryin, _data[i], r._data[i], &_data[i]);
			carryin = carryout;
		}
#else
		for (size_t i = 0; i < n_qw; ++i)
		{
			_data[i] = __builtin_addcll(_data[i], r._data[i], carryin, (unsigned long long *)&carryout);
			carryin = carryout;
		}
#endif
		return *this;
	}
	ZAKey<BITS> &operator-=(const ZAKey<BITS> &r)
	{
		uint64_t carryin = 0, carryout;
#ifdef _MSC_VER
		for (size_t i = 0; i < n_qw; ++i)
		{
			carryout = _subborrow_u64(carryin, _data[i], r._data[i], &_data[i]);
			carryin = carryout;
		}
#else
		for (size_t i = 0; i < n_qw; ++i)
		{
			_data[i] = __builtin_subcll(_data[i], r._data[i], carryin, (unsigned long long *)&carryout);
			carryin = carryout;
		}
#endif
		return *this;
	}
};

template <class T, int BITS>
T createPK(const T &b, const ZAKey<BITS> &key)
{
	T m = b;
	std::array<T, 2> r; //one is the real the other is faked to blur the timing
#ifdef FIXED_TIME_CALC
	r[0].toIdentity(); // the fake
#endif
	r[1].toIdentity(); // the real
	uint64_t ck;
	for (int k = 0; k < ZAKey<BITS>::n_qw; ++k)
	{
		ck = key[k];
		for (int i = 0; i < 64; ++i)
		{
#ifdef FIXED_TIME_CALC
			r[ck & 1] = r[ck & 1] * m;
#else
			if (ck & 1)
			{
				r[1] = r[1] * m;
			}
#endif
			m = m * m;
			ck >>= 1;
		}
	}
	return r[1];
}

template <class T>
void hexprint(const T &x)
{
	BYTE buf[T::size()];
	x.write(buf);
	int i;
	std::cout << std::hex;
	for (i = 0; i < T::size(); ++i)
	{
		std::cout << std::setw(2) << std::setfill('0') << (int)buf[i];
	}
	std::cout << std::setw(1) << std::dec << std::endl;
	// for (i = 0; i < T::size(); ++i)
	// {
	// 	std::cout << (int)buf[i] << " ";
	// }
	// std::cout << std::setw(1) << std::endl;
}
