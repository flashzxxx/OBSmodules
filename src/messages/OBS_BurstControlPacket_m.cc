//
// Generated file, do not edit! Created by nedtool 4.6 from src/messages/OBS_BurstControlPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "OBS_BurstControlPacket_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

Register_Class(OBS_BurstControlPacket);

OBS_BurstControlPacket::OBS_BurstControlPacket(const char *name, int kind) : ::cPacket(name,kind)
{
    this->burstArrivalDelta_var = -1;
    this->burstColour_var = -1;
    this->label_var = -1;
    this->burstifierId_var = -1;
    this->numSeq_var = -1;
    this->senderId_var = -1;
    this->burstSize_var = -1;
}

OBS_BurstControlPacket::OBS_BurstControlPacket(const OBS_BurstControlPacket& other) : ::cPacket(other)
{
    copy(other);
}

OBS_BurstControlPacket::~OBS_BurstControlPacket()
{
}

OBS_BurstControlPacket& OBS_BurstControlPacket::operator=(const OBS_BurstControlPacket& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void OBS_BurstControlPacket::copy(const OBS_BurstControlPacket& other)
{
    this->burstArrivalDelta_var = other.burstArrivalDelta_var;
    this->burstColour_var = other.burstColour_var;
    this->label_var = other.label_var;
    this->burstifierId_var = other.burstifierId_var;
    this->numSeq_var = other.numSeq_var;
    this->senderId_var = other.senderId_var;
    this->burstSize_var = other.burstSize_var;
}

void OBS_BurstControlPacket::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->burstArrivalDelta_var);
    doPacking(b,this->burstColour_var);
    doPacking(b,this->label_var);
    doPacking(b,this->burstifierId_var);
    doPacking(b,this->numSeq_var);
    doPacking(b,this->senderId_var);
    doPacking(b,this->burstSize_var);
}

void OBS_BurstControlPacket::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->burstArrivalDelta_var);
    doUnpacking(b,this->burstColour_var);
    doUnpacking(b,this->label_var);
    doUnpacking(b,this->burstifierId_var);
    doUnpacking(b,this->numSeq_var);
    doUnpacking(b,this->senderId_var);
    doUnpacking(b,this->burstSize_var);
}

simtime_t OBS_BurstControlPacket::getBurstArrivalDelta() const
{
    return burstArrivalDelta_var;
}

void OBS_BurstControlPacket::setBurstArrivalDelta(simtime_t burstArrivalDelta)
{
    this->burstArrivalDelta_var = burstArrivalDelta;
}

int OBS_BurstControlPacket::getBurstColour() const
{
    return burstColour_var;
}

void OBS_BurstControlPacket::setBurstColour(int burstColour)
{
    this->burstColour_var = burstColour;
}

int OBS_BurstControlPacket::getLabel() const
{
    return label_var;
}

void OBS_BurstControlPacket::setLabel(int label)
{
    this->label_var = label;
}

int OBS_BurstControlPacket::getBurstifierId() const
{
    return burstifierId_var;
}

void OBS_BurstControlPacket::setBurstifierId(int burstifierId)
{
    this->burstifierId_var = burstifierId;
}

int OBS_BurstControlPacket::getNumSeq() const
{
    return numSeq_var;
}

void OBS_BurstControlPacket::setNumSeq(int numSeq)
{
    this->numSeq_var = numSeq;
}

int OBS_BurstControlPacket::getSenderId() const
{
    return senderId_var;
}

void OBS_BurstControlPacket::setSenderId(int senderId)
{
    this->senderId_var = senderId;
}

int OBS_BurstControlPacket::getBurstSize() const
{
    return burstSize_var;
}

void OBS_BurstControlPacket::setBurstSize(int burstSize)
{
    this->burstSize_var = burstSize;
}

class OBS_BurstControlPacketDescriptor : public cClassDescriptor
{
  public:
    OBS_BurstControlPacketDescriptor();
    virtual ~OBS_BurstControlPacketDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(OBS_BurstControlPacketDescriptor);

OBS_BurstControlPacketDescriptor::OBS_BurstControlPacketDescriptor() : cClassDescriptor("OBS_BurstControlPacket", "cPacket")
{
}

OBS_BurstControlPacketDescriptor::~OBS_BurstControlPacketDescriptor()
{
}

bool OBS_BurstControlPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<OBS_BurstControlPacket *>(obj)!=NULL;
}

const char *OBS_BurstControlPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int OBS_BurstControlPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
}

unsigned int OBS_BurstControlPacketDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<7) ? fieldTypeFlags[field] : 0;
}

const char *OBS_BurstControlPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "burstArrivalDelta",
        "burstColour",
        "label",
        "burstifierId",
        "numSeq",
        "senderId",
        "burstSize",
    };
    return (field>=0 && field<7) ? fieldNames[field] : NULL;
}

int OBS_BurstControlPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstArrivalDelta")==0) return base+0;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstColour")==0) return base+1;
    if (fieldName[0]=='l' && strcmp(fieldName, "label")==0) return base+2;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstifierId")==0) return base+3;
    if (fieldName[0]=='n' && strcmp(fieldName, "numSeq")==0) return base+4;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderId")==0) return base+5;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstSize")==0) return base+6;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *OBS_BurstControlPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "simtime_t",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
    };
    return (field>=0 && field<7) ? fieldTypeStrings[field] : NULL;
}

const char *OBS_BurstControlPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int OBS_BurstControlPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstControlPacket *pp = (OBS_BurstControlPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string OBS_BurstControlPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstControlPacket *pp = (OBS_BurstControlPacket *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->getBurstArrivalDelta());
        case 1: return long2string(pp->getBurstColour());
        case 2: return long2string(pp->getLabel());
        case 3: return long2string(pp->getBurstifierId());
        case 4: return long2string(pp->getNumSeq());
        case 5: return long2string(pp->getSenderId());
        case 6: return long2string(pp->getBurstSize());
        default: return "";
    }
}

bool OBS_BurstControlPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstControlPacket *pp = (OBS_BurstControlPacket *)object; (void)pp;
    switch (field) {
        case 0: pp->setBurstArrivalDelta(string2double(value)); return true;
        case 1: pp->setBurstColour(string2long(value)); return true;
        case 2: pp->setLabel(string2long(value)); return true;
        case 3: pp->setBurstifierId(string2long(value)); return true;
        case 4: pp->setNumSeq(string2long(value)); return true;
        case 5: pp->setSenderId(string2long(value)); return true;
        case 6: pp->setBurstSize(string2long(value)); return true;
        default: return false;
    }
}

const char *OBS_BurstControlPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    };
}

void *OBS_BurstControlPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstControlPacket *pp = (OBS_BurstControlPacket *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


