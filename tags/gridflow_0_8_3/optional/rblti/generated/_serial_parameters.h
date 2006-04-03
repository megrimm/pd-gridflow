#ifndef _Rserial_parameters_h
#define _Rserial_parameters_h

namespace lti {
namespace _serial {
class Rserial_parameters : public lti::ioObject
{
public:
    enum ePortType {
Com1  = 0 
,Com2 
,Com3 
,Com4 
,Com5 
,Com6 
,Com7 
,Com8 
};

    enum eBaudRateType {
Baud0 
,Baud300 
,Baud600 
,Baud1200 
,Baud1800 
,Baud2400 
,Baud4800 
,Baud9600 
,Baud19200 
,Baud38400 
,Baud57600 
,Baud76800 
,Baud115200 
};

    enum eCharBitSizeType {
Cs4  = 4 
,Cs5 
,Cs6 
,Cs7 
,Cs8 
};

    enum eStopBitType {
One 
,OneFive 
,Two 
};

    enum eParityType {
No 
,Even 
,Odd 
,Space 
,Mark 
};

     Rserial_parameters (  );

     Rserial_parameters ( const Rserial_parameters &  arg0 );

    virtual ~Rserial_parameters (  );

    const char * getTypeName (  ) const;

    Rserial_parameters & copy ( const Rserial_parameters &  arg0 );

    Rserial_parameters & operator= ( const Rserial_parameters &  arg0 );

    virtual Rserial_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eBaudRateType baudRate  ;
    ePortType port  ;
    eCharBitSizeType characterSize  ;
    eParityType parity  ;
    eStopBitType stopBits  ;
    int receiveTimeout  ;
};
}
typedef lti::_serial::Rserial_parameters serial_parameters;
}
#endif

