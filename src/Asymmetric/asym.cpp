#include <array>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <endian.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;

int my_rand()// some stupid random function for proof of concept.
{
    static int a =4;
    static int state=25326;
    state= ~(a*state+90877);
    a= (state^a)+15;
    a=a<<5;
    a^=state;
    state>>=3;
    return state;
}
class ZAObject
{
public:
    virtual void read(const BYTE * buf){}
    virtual void write(BYTE * buf)const{}
	virtual void print()const=0;
	virtual void randomize()=0;
	virtual void toIdentity()=0;
	virtual void toZero()=0;
};

template <typename T, typename TL, DWORD P >
class ZAIntegerModP : public ZAObject
{
	static_assert(sizeof(TL)>=2*sizeof(T),"TL must be at least twice in size than T");
public:
    ZAIntegerModP():_val(0){}
    inline static size_t size()
	{
	    return sizeof(T);
	}
    virtual void read(const BYTE * buf){
	if (sizeof(T)==1)
		_val=buf[0];
	if (sizeof(T)==2)
		_val=le16toh(*(uint16_t*)buf);
	if (sizeof(T)==4)
		_val=le32toh(*(uint32_t*)buf);
	if (sizeof(T)==8)
                _val=le64toh(*(uint64_t*)buf);
	}
    virtual void write(BYTE * buf)const{
	if (sizeof(T)==1)
                buf[0]=_val;
        if (sizeof(T)==2)
                *(uint16_t*)buf=htole16(_val);
        if (sizeof(T)==4)
                *(uint32_t*)buf=htole32(_val);
        if (sizeof(T)==8)
                *(uint64_t*)buf=htole64(_val);
	}
	virtual void print()const
	{
	    std::cout<<(uint64_t)_val;
	}
	virtual void randomize()
	{
	    _val = my_rand()%P;
	}
	virtual void toIdentity()
	{
	    _val = 1;
	}
	virtual void toZero()
	{
	    _val = 0;
	}
	friend inline const ZAIntegerModP<T,TL,P>  operator*(const ZAIntegerModP<T,TL,P>& l,const ZAIntegerModP<T,TL,P>& r)
	{
		ZAIntegerModP<T,TL,P> m;
		m._val=((TL)l._val*r._val)%P;
		return m;
	}
	friend inline const ZAIntegerModP<T,TL,P>  operator+(const ZAIntegerModP<T,TL,P>& l,const ZAIntegerModP<T,TL,P>& r)
	{
		ZAIntegerModP<T,TL,P> m;
		m._val=((TL)l._val+r._val)%P;
		return m;
	}
	friend inline const ZAIntegerModP<T,TL,P>  operator-(const ZAIntegerModP<T,TL,P>& l,const ZAIntegerModP<T,TL,P>& r)
	{
		ZAIntegerModP<T,TL,P> m;
		m._val=(P+(TL)l._val-r._val)%P;
		return m;
	}
	ZAIntegerModP<T,TL,P> & operator+=(const ZAIntegerModP<T,TL,P>& r)
	{
		_val=((TL)_val+r._val)%P;
		return *this;
	}
private:
	T _val;
};
template <typename T>
class ZAInteger : public ZAObject
{
public:
    ZAInteger():_val(0){}
    inline static size_t size()
	{
	    return sizeof(T);
	}
    virtual void read(const BYTE * buf){
	if (sizeof(T)==1)
		_val=buf[0];
	if (sizeof(T)==2)
		_val=le16toh(*(uint16_t*)buf);
	if (sizeof(T)==4)
		_val=le32toh(*(uint32_t*)buf);
	if (sizeof(T)==8)
                _val=le64toh(*(uint64_t*)buf);
	}
    virtual void write(BYTE * buf)const{
	if (sizeof(T)==1)
                buf[0]=_val;
        if (sizeof(T)==2)
                *(uint16_t*)buf=htole16(_val);
        if (sizeof(T)==4)
                *(uint32_t*)buf=htole32(_val);
        if (sizeof(T)==8)
                *(uint64_t*)buf=htole64(_val);
	}
	virtual void print()const
	{
	    std::cout<<(uint64_t)_val;
	}
	virtual void randomize()
	{
	    _val = my_rand();
	}
	virtual void toIdentity()
	{
	    _val = 1;
	}
	virtual void toZero()
	{
	    _val = 0;
	}
		friend inline const ZAInteger<T>  operator*(const ZAInteger<T>& l,const ZAInteger<T>& r)
	{
		ZAInteger<T> m;
		m._val=l._val*r._val;
		return m;
	}
	friend inline const ZAInteger<T>  operator+(const ZAInteger<T>& l,const ZAInteger<T>& r)
	{
		ZAInteger<T> m;
		m._val=l._val+r._val;
		return m;
	}
	friend inline const ZAInteger<T>  operator-(const ZAInteger<T>& l,const ZAInteger<T>& r)
	{
		ZAInteger<T> m;
		m._val=l._val-r._val;
		return m;
	}
	ZAInteger<T> & operator+=(const ZAInteger<T>& r)
	{
	
		_val+=r._val;
		return *this;
	}
private:
	T _val;
};
template <class T>
class ZAComplex : public ZAObject
{
public:
	ZAComplex(){}
	friend inline const ZAComplex<T>  operator*(const ZAComplex<T>& l,const ZAComplex<T>& r)
	{
		ZAComplex<T> m;
		m._real = l._real*r._real - l._img*r._img;
		m._img =  l._real*r._img + l._img*r._real;
		return m;
	}
	friend inline const ZAComplex<T>  operator+(const ZAComplex<T>& l,const ZAComplex<T>& r)
	{
		ZAComplex<T> m;
		m._real = l._real + r._real;
		m._img =  l._img + r._img;
		return m;
	}
	ZAComplex<T>&  operator+=(const ZAComplex<T>& r)
	{
		
		_real +=  r._real;
		_img +=  r._img;
		return *this;
	}
	inline static size_t size()
	{
	    return 2*T::size();
	}
	virtual void read(const BYTE* buf){
		_real.read(buf);
		buf+=T::size();
		_img.read(buf);
	}
	virtual void write(BYTE* buf)const{
                _real.write(buf);
                buf+=T::size();
                _img.write(buf);
        }
	virtual void print()const
	{
	    std::cout <<"<r:";
	    _real.print();
	    std::cout <<",i:";
	    _img.print();
	    std::cout <<">";
	}
	virtual void randomize()
	{
	    _real.randomize();
	    _img.randomize();
	}
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
private:
	T _real,_img;
};

template <class T, size_t ... Dims>
class ZAMultiDimTable: public ZAObject
{
public:
	static constexpr size_t nDims = sizeof...(Dims);
	static constexpr size_t nElements = (... * Dims);
	static constexpr std::array<size_t, nDims> _indices = {Dims...};
private:
	static constexpr size_t addIndices(size_t ind1,size_t ind2)
	{
	    size_t ret = 0;
	    size_t mul = 1;
	    for (size_t i=0; i< nDims;++i)
	    {
	        ret+=mul*((ind1+ind2)%_indices[i]);
	        ind1/=_indices[i];
	        ind2/=_indices[i];
	        mul*=_indices[i];
	    }
	    return ret;
	}
	using Elem2d = std::array<std::array<size_t,nElements>,nElements>;
	//typedef size_t Elem2d[nElements][nElements];
	static constexpr Elem2d initializeIndicesConvert()
	{
		Elem2d conv_map=Elem2d();
		for (size_t i=0; i< nElements; ++i)
		{
			for(size_t j=0; j< nElements;++j)
			{
				conv_map[i][j]=addIndices(i,j);
			}
		}
		return conv_map;
	}
public:
	static constexpr Elem2d conv_map  {initializeIndicesConvert()};
	friend inline const ZAMultiDimTable<T, Dims...>  operator*(const ZAMultiDimTable<T, Dims...>& l,const ZAMultiDimTable<T, Dims...>& r)
	{
		ZAMultiDimTable<T, Dims...> m;
		for(size_t i=0; i < nElements; ++i)
		{
		    for (size_t j = 0; j < nElements; ++j)
		    {
		        m._table[conv_map[i][j]]+=l._table[i]*r._table[j];
			//m._table[addIndices(i,j)]+=l._table[i]*r._table[j];
		    }
		}
		return m;
	}
	friend inline const ZAMultiDimTable<T, Dims...>  operator+(const ZAMultiDimTable<T, Dims...>& l,const ZAMultiDimTable<T, Dims...>& r)
	{
		ZAMultiDimTable<T, Dims...> m;
		for(size_t i=0; i < nElements; ++i)
		{
		    m._table[i] = l._table[i] + r._table[i];
		}
		return m;
	}
	ZAMultiDimTable<T, Dims...>&  operator+=(const ZAMultiDimTable<T, Dims...>& r)
	{
		for(size_t i=0; i < nElements; ++i)
		{
		    _table[i] += r._table[i];
		}
		return *this;
	}
	
	inline static size_t size()
	{
	    return nElements*T::size();
	}
	virtual void read(const BYTE* buf){
		for (size_t i=0;i<nElements;++i)
		{
                _table[i].read(buf);
                buf+=T::size();
                }
        }
        virtual void write(BYTE* buf)const{
		for (size_t i=0;i<nElements;++i)
                {
                _table[i].write(buf);
                buf+=T::size();
                }

        }
	virtual void print()const
	{
        std::cout << "MultiDim"<< "{";
        for (size_t i=0; i< nElements;++i)
        {
            if (i>0)
                std::cout<<", ";
            _table[i].print();

        }
        std::cout<<"}";
	}
	virtual void randomize()
	{
	    for (size_t i=0; i< nElements; ++i)
        {
            _table[i].randomize();
        }
	}
	virtual void toIdentity()
	{
	    _table[0].toIdentity();
	    for (size_t i=1; i< nElements; ++i)
        {
            _table[i].toZero();
        }
	}
	virtual void toZero()
	{
	    for (size_t i=0; i< nElements; ++i)
        {
            _table[i].toZero();
        }
	}
private:
	std::array<T, nElements > _table;
};

template <class T, size_t S>
class ZASqMatrix : public ZAObject
{
public:
	friend inline const ZASqMatrix<T, S>  operator*(const ZASqMatrix<T, S>& l,const ZASqMatrix<T, S>& r)
	{
		ZASqMatrix<T, S> m;
		for(size_t i=0; i < S; ++i)
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
	friend inline const ZASqMatrix<T, S>  operator+(const ZASqMatrix<T, S>& l,const ZASqMatrix<T, S>& r)
	{
		ZASqMatrix<T, S> m;
		for(size_t i=0; i < S; ++i)
		{
		    for (size_t j = 0; j < S; ++j)
		    {
		            m._table[i][j] = l._table[i][j]+r._table[i][j];
		    }
		}
		return m;
	}
	ZASqMatrix<T, S>&  operator+=(const ZASqMatrix<T, S>& r)
	{
		for(size_t i=0; i < S; ++i)
		{
		    for (size_t j = 0; j < S; ++j)
		    {
		            _table[i][j] += r._table[i][j];
		    }
		}
        return *this;
	}
	inline static size_t size()
	{
	    return S*S*T::size();
	}
	virtual void read(const BYTE* buf){
                for (size_t i=0;i<S;++i)
                {
			for (size_t j=0; j< S;++j)
                	{
		_table[i][j].read(buf);
                buf+=T::size();
			}
                }
        }
        virtual void write(BYTE* buf)const{
		for (size_t i=0;i<S;++i)
                {
                        for (size_t j=0; j< S;++j)
                        {
                _table[i][j].write(buf);
                buf+=T::size();
                        }
                }
        }
	virtual void print()const
	{
        std::cout << "Mat"<< "[";
        for (size_t i=0; i< S;++i)
        {
            if (i>0)
                std::cout<<";"<<std::endl;
            for (size_t j=0; j< S;++j)
            {
                if (j>0)
                    std::cout<<", ";
                _table[i][j].print();

            }

        }
        std::cout<<"]"<<std::endl;
	}
    virtual void randomize()
	{
        for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
            {
                _table[i][j].randomize();
            }

        }
	}
	virtual void toIdentity()
	{
        for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
            {
                if (i==j)
                    _table[i][j].toIdentity();
                else
                    _table[i][j].toZero();
            }

        }
	}
	virtual void toZero()
	{
        for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
            {
                _table[i][j].toZero();
            }

        }
	}
private:
    std::array<std::array<T, S >,S> _table;
};

template<class T, size_t S>
class ZACubeMatrix
{
private:
	std::array<std::array<std::array<T, S >, S >,S> _cube;
public:
	inline static size_t size()
        {
            return S*S*S*T::size();
        }	
        friend inline ZACubeMatrix<T, S>  operator*(const ZACubeMatrix<T, S>& l,const ZACubeMatrix<T, S>& r)
	{
		ZACubeMatrix<T, S> m;
		for(size_t i=0; i < S; ++i)
		{
		    for (size_t j = 0; j < S; ++j)
		    {
		        for (size_t k = 0; k < S; ++k)
		        {
	for (size_t p = 0; p < S; ++p)
    {
		 //for (size_t q = 0; q < S; ++q)
		        {
		/*            m._cube[i][j][k] += 
l._cube[i][p][q] * r._cube[q][j][p]+
l._cube[q][j][p] * r._cube[p][q][k]+
l._cube[p][q][k] * r._cube[i][p][q] ;*/
m._cube[i][j][k] += 
(l._cube[i][j][p] * r._cube[p][j][k]+
 l._cube[i][p][k] * r._cube[p][j][k]+
 l._cube[i][j][p] * r._cube[i][p][k]) ;
		        }
    }
                }

		    }
		}
		return m;
	}
	friend inline ZACubeMatrix<T, S>  operator+(const ZACubeMatrix<T, S>& l,const ZACubeMatrix<T, S>& r)
	{
		ZACubeMatrix<T, S> m;
		for(size_t i=0; i < S; ++i)
		{
		    for (size_t j = 0; j < S; ++j)
		    {
			for (size_t k = 0; k < S; ++k)
		            m._cube[i][j][k] = l._cube[i][j][k]+r._cube[i][j][k];
		    }
		}
		return m;
	}
	ZACubeMatrix<T, S>&  operator+=(const ZACubeMatrix<T, S>& r)
	{
		
		for(size_t i=0; i < S; ++i)
		{
		    for (size_t j = 0; j < S; ++j)
		    {
			for (size_t k = 0; k < S; ++k)
		            _cube[i][j][k] += r._cube[i][j][k];
		    }
		}
		return *this;
	}
	virtual void read(const BYTE* buf){
		        for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
		    for (size_t k = 0; k < S; ++k)
            {
                    _cube[i][j][k].read(buf);
                    buf+=T::size();
            }
        }
	}
	virtual void write(BYTE* buf)const{
                       for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
		    for (size_t k = 0; k < S; ++k)
            {
                    _cube[i][j][k].write(buf);
                    buf+=T::size();
            }
        }
        }
        virtual void randomize(){
                       for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
		    for (size_t k = 0; k < S; ++k)
            {
                    _cube[i][j][k].randomize();
            }
        }
        }

	virtual void toIdentity()
	{
        for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
		    for (size_t k = 0; k < S; ++k)
            {
                if (i==j && j==k)
                    _cube[i][j][k].toIdentity();
                else
                    _cube[i][j][k].toZero();
            }

        }
	}
	virtual void toZero()
	{
        for (size_t i=0; i< S;++i)
        {
            for (size_t j=0; j< S;++j)
		    for (size_t k = 0; k < S; ++k)
            {
                    _cube[i][j][k].toZero();
            }

        }
	}
};


template <class T,int K>
T createPK(const T& b, uint64_t key[K])
{
    T m=b;
    std::array<T,2> r;//one is the real the other is faked to blur the timing
    r[0].toIdentity();// the fake
    r[1].toIdentity();// the real
    uint64_t ck;
    for (int k = 0; k< K; ++k)
    {
        ck = key[k];
        for (int i=0;i<64;++i)
        {
            r[ck&1]=r[ck&1]*m;
            m=m*m;
            ck>>=1;
        }
    }
    return r[1];
}
template <class T>
void hexprint(const T& x)
{
	BYTE buf[T::size()];
	x.write(buf);
	int i;
	std::cout<<std::hex;
	for (i=0; i< T::size(); ++i)
	{
		std::cout<<std::setw(2) << std::setfill('0')<<(int)buf[i];
	}
	std::cout<<std::dec<<std::endl;
	for (i=0; i< T::size(); ++i)
	{
		std::cout<<(int)buf[i]<<" ";
	}
	std::cout<<std::setw(1)<<std::endl;
}
int main()
{
    typedef ZASqMatrix<ZAMultiDimTable<ZAComplex<ZAIntegerModP<uint16_t,uint32_t,65521> >,2,3,5>,3> Asym_t;
    std::ios::sync_with_stdio(false);
    Asym_t a;
    typedef ZACubeMatrix<ZAInteger<uint8_t>,2> Asym3d_t;
    Asym3d_t t;
    t.toIdentity();
    hexprint(t);
    t=t*t;
    hexprint(t);
    a.randomize();
    uint64_t key1[4] = {53263675,35216858,4743727,8695};
    uint64_t key2[4] = {32423234,23415,111111111,42513666};
    Asym_t pk1,pk2;
    pk1=createPK<Asym_t,4>(a,key1);
    pk2=createPK<Asym_t,4>(a,key2);
    std::cout<<"base:";
hexprint(a);
 //   a.print();
    std::cout<<"pk1:";
hexprint(pk1);
//    pk1.print();
    std::cout<<"pk2:";
hexprint(pk2);
//    pk2.print();
    Asym_t sec1,sec2;
    sec1=createPK<Asym_t,4>(pk2,key1);
    sec2=createPK<Asym_t,4>(pk1,key2);
    std::cout<<"sec1:";
    hexprint(sec1);
//    sec1.print();
    std::cout<<"sec2:";
    hexprint(sec2);
//    sec2.print();
    std::cout<<"size:" << Asym_t::size()<<std::endl;
    return 0;
}

