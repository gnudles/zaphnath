#include <array>
#include <iostream>
#include <stdint.h>

int my_rand()
{
    static int a =4;
    static int state=25326;
    state= a*state+90877;
    a= state^a+15;
    a=a<<5;
    a^=state;
    state>>=3;
    return state;
}
class ZAObject
{
public:
    virtual void read(char * buf){}
    virtual void write(char * buf){}
	virtual void print()=0;
	virtual void randomize()=0;
	virtual void toIdentity()=0;
	virtual void toZero()=0;
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
    virtual void read(char * buf){}
    virtual void write(char * buf){}
	virtual void print()
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
	inline static size_t size()
	{
	    return 2*T::size();
	}
	virtual void print()
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

	static size_t addIndices(size_t ind1,size_t ind2)
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

	friend inline const ZAMultiDimTable<T, Dims...>  operator*(const ZAMultiDimTable<T, Dims...>& l,const ZAMultiDimTable<T, Dims...>& r)
	{
		ZAMultiDimTable<T, Dims...> m;
		for(size_t i=0; i < nElements; ++i)
		{
		    for (size_t j = 0; j < nElements; ++j)
		    {
		        m._table[addIndices(i,j)]=m._table[addIndices(i,j)]+l._table[i]*r._table[j];
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
	inline static size_t size()
	{
	    return nElements*T::size();
	}
	virtual void print()
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
		            m._table[i][j] = m._table[i][j] + l._table[i][k] * r._table[k][j];
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
		            m._table[i][j] += l._table[i][j]+r._table[i][j];
		    }
		}
		return m;
	}
	inline static size_t size()
	{
	    return S*S*T::size();
	}
	virtual void print()
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
template <class T,int K>
T createPK(const T& b, uint64_t key[K])
{
    T m=b;
    T r;
    r.toIdentity();
    uint64_t ck;
    for (int k = 0; k< K; ++k)
    {
        ck = key[k];
        for (int i=0;i<64;++i)
        {
            if (ck&1)
            {
                r=r*m;
            }
            m=m*m;
            ck>>=1;
        }
    }
    return r;
}
int main()
{
    typedef ZASqMatrix<ZAMultiDimTable<ZAComplex<ZAInteger<uint8_t> >,2,3>,5> Asym_t;
    std::ios::sync_with_stdio(false);
    Asym_t a;
    a.randomize();
    uint64_t key1[4] = {53263675,35216858,4743727,8695};
    uint64_t key2[4] = {32423234,23415,111111111,42513666};
    Asym_t pk1,pk2;
    pk1=createPK<Asym_t,4>(a,key1);
    pk2=createPK<Asym_t,4>(a,key2);
    std::cout<<"base:";
    a.print();
    std::cout<<"pk1:";
    pk1.print();
    std::cout<<"pk2:";
    pk2.print();
    Asym_t sec1,sec2;
    sec1=createPK<Asym_t,4>(pk2,key1);
    sec2=createPK<Asym_t,4>(pk1,key2);
    std::cout<<"sec1:";
    sec1.print();
    std::cout<<"sec2:";
    sec2.print();
    std::cout<<"size:" << Asym_t::size()<<std::endl;
    return 0;
}
