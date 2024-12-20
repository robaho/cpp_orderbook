#pragma once

#include <cstdlib>
#include <string_view>
#include <charconv>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <cmath>
#include <cstring>

constexpr int pow10(uint32_t exp)
{
    int total = 1;
    while(exp--) total*=10;
    return total;
}

template<int nPlaces=7>
class Fixed {
private:
    constexpr static const int64_t scale = int64_t(pow10(nPlaces));
    constexpr static const double MAX = 999999999999999999.0 / scale;
    constexpr static const int64_t nan = (int64_t)((1ULL<<63)-1);

    constexpr static std::string_view zeros() {
        const char* _zeros = "0.0000000000000000";
        return std::basic_string_view(_zeros,2+nPlaces);
    }

    int64_t fp;
    std::string_view toStr(char *buffer, int &decimal) const {
        if(fp==nan) {
            decimal=-1;
            return "NaN";
        }
        if(fp==0) {
            decimal=1;
            return zeros();
        }
        return itoa(buffer,fp,decimal);
    }
    static std::string_view itoa(char *buf,int64_t val,int &decimal) {
        bool neg = val < 0;

        if(neg) {
            val = val * -1;
        }

        buf[BUFFER_SIZE-1]=0;
        int i = BUFFER_SIZE-1-1;
        int idec = i - nPlaces;

        for(;val >= 10 || i >= idec;) {
            buf[i] = val%10 + '0';
            i--;
            if (i == idec) {
                buf[i] = '.';
                i--;
            }
            val /= 10;
        }
        buf[i] = val + '0';
        if(neg) {
            i--;
            buf[i] = '-';
        }
        decimal = idec-i;
        return buf+i;
    }

    inline explicit Fixed(int64_t raw) {
        fp=raw;
    }
public:
    constexpr static const int BUFFER_SIZE = 24;
    const static Fixed<nPlaces> NaN() {
        return Fixed((int64_t)nan);
    }

    Fixed(const char *s) {
        const char* exp = nullptr;
        for(auto *cp=s;*cp;cp++) {
            if(*cp=='E' || *cp=='e') {
                exp = cp;
                break;
            }
        }
        if(strcmp(s,"NaN")==0) {
            fp = nan;
            return;
        }
        const char *decimal = strchr(s,'.');
        int64_t i = atoll(s);
        int64_t f = 0;
        int64_t sign = 1;
        if(decimal) {
            auto cp = decimal + 1;
            f = 0;
            for(int p = 0; p<nPlaces;p++) {
                if(*cp==0 || *cp=='E' || *cp=='e') f*=10;
                else f = f*10 + (*cp++)-'0';
            }
        }
        if(i<0) {
            sign=-1;
            i*=-1;
        }
        fp = (sign * (i*scale +f));
        if(exp) {
            auto _exp = atoll(exp+1);
            if(_exp>=0)
                fp *= pow10(std::abs(_exp));
            else
                fp /= pow10(std::abs(_exp));
        }
    }
    Fixed(std::string_view s) {
        const char* end = s.data()+s.length();
        const char* exp = nullptr;
        for(auto *cp=s.data();cp!=end;cp++) {
            if(*cp=='E' || *cp=='e') {
                exp = cp;
                break;
            }
        }
        if(s=="NaN") {
            fp = nan;
            return;
        }
        auto decimal = s.find('.');
        int64_t i;
        std::from_chars(s.data(),end,i);
        int64_t f = 0;
        int64_t sign = 1;
        if(decimal>=0) {
            auto cp = s.data() + decimal + 1;
            f = 0;
            for(int p = 0; p<nPlaces;p++) {
                if(cp==exp || cp==end) f*=10;
                else f = f*10 + (*cp++)-'0';
            }
        }
        if(i<0) {
            sign=-1;
            i*=-1;
        }
        fp = (sign * (i*scale +f));
        if(exp) {
            int _exp;
            std::from_chars(exp+1,end,_exp);
            if(_exp>=0)
                fp *= pow10(std::abs(_exp));
            else
                fp /= pow10(std::abs(_exp));
        }
    }
    Fixed(double f) {
        if(std::isnan(f)) {
            fp = nan;
        } else {
            if(f >= MAX || f <= -MAX) {
                fp = nan;
            } else {
                double round = f >= 0 ? 0.5 : -0.5;
                fp = (f*scale +round);
            }
        }
    }
    Fixed(int i) {
        fp = i * scale;
    }
    Fixed(int64_t i,int n) {
        if(n > nPlaces) {
            i = i / (pow10(n-nPlaces));
            n = nPlaces;
        }
        fp = i * pow10(nPlaces-n);
    }
    inline bool isNaN() const { return fp == nan; }
    inline bool isZero() const { return fp == 0; }
    inline Fixed(const Fixed& other) {
        fp = other.fp;
    }
    inline Fixed(Fixed&& other) {
        fp = other.fp;
    }

    inline explicit operator double() const {
        return ((double)fp) / (double)scale;
    }

    inline bool operator==(const Fixed& other) const {
        return fp == other.fp;
    }
    bool operator==(const char* s) const {
        return fp == Fixed(s).fp;
    }
    inline Fixed& operator=(const Fixed& other) {
        if(fp==nan || other.fp==nan) {
            fp = nan;
        } else {
            fp = other.fp;
        }
        return *this;
    }
    inline Fixed& operator=(Fixed&& other) {
        if(fp==nan || other.fp==nan) {
            fp = nan;
        } else {
            fp = other.fp;
        }
        return *this;
    }

    inline Fixed operator+(const Fixed& other) const {
        if(fp==nan || other.fp==nan) {
            return Fixed((int64_t)nan);
        } else {
            return Fixed((int64_t)(fp + other.fp));
        }
    }
    inline Fixed operator-(const Fixed& other) const {
        if(fp==nan || other.fp==nan) {
            return Fixed((int64_t)nan);
        } else {
            return Fixed((int64_t)(fp - other.fp));
        }
        return *this;
    }
    inline Fixed operator*(const Fixed& other) const {
        if(fp==nan || other.fp==nan) {
            return Fixed((int64_t)nan);
        }
        auto fp_a = fp / scale;
        auto fp_b = fp % scale;

        auto fp0_a = other.fp / scale;
        auto fp0_b = other.fp % scale;

        auto _sign = sign()*other.sign();

        int64_t result=0;
        if (fp0_a != 0) {
            result = fp_a*fp0_a*scale + fp_b*fp0_a;
        }
        if (fp0_b != 0) {
            result = result + (fp_a * fp0_b) + ((fp_b)*fp0_b+5*_sign*(scale/10))/scale;
        }
        return Fixed((int64_t)result);
    }
    inline Fixed operator/(const Fixed& other) const {
        auto d1 = operator double();
        auto d2 = double(other);
        return Fixed(d1/d2);
    }
    inline bool operator<(const Fixed& other) const {
        return cmp(other) < 0;
    }
    inline bool operator<=(const Fixed& other) const {
        return cmp(other) <= 0;
    }
    inline bool operator>(const Fixed& other) const {
        return cmp(other) > 0;
    }
    inline bool operator>=(const Fixed& other) const {
        return cmp(other) >= 0;
    }

    inline int cmp(const Fixed& other) const {
        if(fp == other.fp) {
            return 0;
        }
        if(isNaN()) {
            return 1;
        }
        if(other.isNaN()) {
            return -1;
        }
        if(fp<other.fp){
            return -1;
        }
        return 1;
    }

    int sign() const {
        return cmp(0);
    }

    Fixed abs() const {
        if(fp==nan) return *this;
        if(sign()<0) {
            return Fixed((int64_t)(fp * -1));
        }
        return *this;
    }

    int64_t intPart() const {
        if(isNaN()) return 0;
        return fp / scale;
    }

    double fracPart() const {
        if(isNaN()) return 0;
        return (fp % scale)/(double)scale;
    }

    Fixed round(const int n) const {
        if(isNaN()) return Fixed(nan);
        int64_t fraction = fp % scale;
        int64_t f0 = fraction / int64_t(pow(10,nPlaces-n-1));
        int digit = std::abs(f0%10);
        f0 = (f0/10);
        if(digit>=5) {
            f0 += 1 * sign();
        }
        f0 = f0 * int64_t(pow(10,nPlaces-n));
        auto int_part = fp -fraction;
        auto fp = int_part + f0;
        return Fixed((int64_t)fp);
    }

    operator std::string() const {
        char buffer[24];
        int point;
        auto s = toStr(buffer,point);
        if(point==-1) return std::string(s);
        int index = s.length() - 1;
        for(;index!=point;index--) {
            if(s[index]!='0') {
                return std::string(s.substr(0,index+1));
            }
        }
        return std::string(s.substr(0,point));
    }
    std::string stringN(int decimals) {
        char buffer[BUFFER_SIZE];
        int point;
        auto s = toStr(buffer,point);
        if(point==-1) return std::string(s);
        if(decimals==0) return std::string(s.substr(0,point));
        return std::string(s.substr(0,point+decimals+1));
    }

    /** write Fixed as string into buffer. buffer must be at least BUFFER_SIZE. */
    void str(char *buffer) {
        char tmp[BUFFER_SIZE];

        int point;
        auto s = toStr(tmp,point);
        if(point==-1) { 
            memcpy(buffer,s.data(),s.length());
            buffer[s.length()]=0;
            return; 
        };
        int index = s.length() - 1;
        for(;index!=point;index--) {
            if(s[index]!='0') {
                memcpy(buffer,s.data(),index+1);
                buffer[index+1]=0;
                return;
            }
        }
        memcpy(buffer,s.data(),point);
        buffer[point]=0;
    }
};

template <int nPlaces=7>
inline std::ostream& operator<<(std::ostream& os, const Fixed<nPlaces>& f)
{
    return os << std::string(f);
}
