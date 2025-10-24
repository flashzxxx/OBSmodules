//
// Generated file, do not edit! Created by nedtool 4.6 from src/messages/OBS_Burst.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "OBS_Burst_m.h"

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

OBS_Burst_Base::OBS_Burst_Base(const char *name, int kind) : ::cPacket(name,kind)
{
    take(&(this->messages_var));
    this->numPackets_var = -1;
    this->minOffset_var = -1;
    this->maxOffset_var = -1;
    this->burstifierId_var = -1;
    this->numSeq_var = -1;
    this->senderId_var = -1;
}

OBS_Burst_Base::OBS_Burst_Base(const OBS_Burst_Base& other) : ::cPacket(other)
{
    take(&(this->messages_var));
    copy(other);
}

OBS_Burst_Base::~OBS_Burst_Base()
{
    drop(&(this->messages_var));
}

OBS_Burst_Base& OBS_Burst_Base::operator=(const OBS_Burst_Base& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void OBS_Burst_Base::copy(const OBS_Burst_Base& other)
{
    this->messages_var = other.messages_var;
    this->messages_var.setName(other.messages_var.getName());
    this->numPackets_var = other.numPackets_var;
    this->minOffset_var = other.minOffset_var;
    this->maxOffset_var = other.maxOffset_var;
    this->burstifierId_var = other.burstifierId_var;
    this->numSeq_var = other.numSeq_var;
    this->senderId_var = other.senderId_var;
}

void OBS_Burst_Base::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->messages_var);
    doPacking(b,this->numPackets_var);
    doPacking(b,this->minOffset_var);
    doPacking(b,this->maxOffset_var);
    doPacking(b,this->burstifierId_var);
    doPacking(b,this->numSeq_var);
    doPacking(b,this->senderId_var);
}

void OBS_Burst_Base::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->messages_var);
    doUnpacking(b,this->numPackets_var);
    doUnpacking(b,this->minOffset_var);
    doUnpacking(b,this->maxOffset_var);
    doUnpacking(b,this->burstifierId_var);
    doUnpacking(b,this->numSeq_var);
    doUnpacking(b,this->senderId_var);
}

cQueue& OBS_Burst_Base::getMessages()
{
    return messages_var;
}

void OBS_Burst_Base::setMessages(const cQueue& messages)
{
    this->messages_var = messages;
}

int OBS_Burst_Base::getNumPackets() const
{
    return numPackets_var;
}

void OBS_Burst_Base::setNumPackets(int numPackets)
{
    this->numPackets_var = numPackets;
}

simtime_t OBS_Burst_Base::getMinOffset() const
{
    return minOffset_var;
}

void OBS_Burst_Base::setMinOffset(simtime_t minOffset)
{
    this->minOffset_var = minOffset;
}

simtime_t OBS_Burst_Base::getMaxOffset() const
{
    return maxOffset_var;
}

void OBS_Burst_Base::setMaxOffset(simtime_t maxOffset)
{
    this->maxOffset_var = maxOffset;
}

int OBS_Burst_Base::getBurstifierId() const
{
    return burstifierId_var;
}

void OBS_Burst_Base::setBurstifierId(int burstifierId)
{
    this->burstifierId_var = burstifierId;
}

int OBS_Burst_Base::getNumSeq() const
{
    return numSeq_var;
}

void OBS_Burst_Base::setNumSeq(int numSeq)
{
    this->numSeq_var = numSeq;
}

int OBS_Burst_Base::getSenderId() const
{
    return senderId_var;
}

void OBS_Burst_Base::setSenderId(int senderId)
{
    this->senderId_var = senderId;
}

class OBS_BurstDescriptor : public cClassDescriptor
{
  public:
    OBS_BurstDescriptor();
    virtual ~OBS_BurstDescriptor();

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

Register_ClassDescriptor(OBS_BurstDescriptor);

OBS_BurstDescriptor::OBS_BurstDescriptor() : cClassDescriptor("OBS_Burst", "cPacket")
{
}

OBS_BurstDescriptor::~OBS_BurstDescriptor()
{
}

bool OBS_BurstDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<OBS_Burst_Base *>(obj)!=NULL;
}

const char *OBS_BurstDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"customize")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int OBS_BurstDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
}

unsigned int OBS_BurstDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND | FD_ISCOBJECT | FD_ISCOWNEDOBJECT,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<7) ? fieldTypeFlags[field] : 0;
}

const char *OBS_BurstDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "messages",
        "numPackets",
        "minOffset",
        "maxOffset",
        "burstifierId",
        "numSeq",
        "senderId",
    };
    return (field>=0 && field<7) ? fieldNames[field] : NULL;
}

int OBS_BurstDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='m' && strcmp(fieldName, "messages")==0) return base+0;
    if (fieldName[0]=='n' && strcmp(fieldName, "numPackets")==0) return base+1;
    if (fieldName[0]=='m' && strcmp(fieldName, "minOffset")==0) return base+2;
    if (fieldName[0]=='m' && strcmp(fieldName, "maxOffset")==0) return base+3;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstifierId")==0) return base+4;
    if (fieldName[0]=='n' && strcmp(fieldName, "numSeq")==0) return base+5;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderId")==0) return base+6;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *OBS_BurstDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "cQueue",
        "int",
        "simtime_t",
        "simtime_t",
        "int",
        "int",
        "int",
    };
    return (field>=0 && field<7) ? fieldTypeStrings[field] : NULL;
}

const char *OBS_BurstDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int OBS_BurstDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    OBS_Burst_Base *pp = (OBS_Burst_Base *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string OBS_BurstDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_Burst_Base *pp = (OBS_Burst_Base *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getMessages(); return out.str();}
        case 1: return long2string(pp->getNumPackets());
        case 2: return double2string(pp->getMinOffset());
        case 3: return double2string(pp->getMaxOffset());
        case 4: return long2string(pp->getBurstifierId());
        case 5: return long2string(pp->getNumSeq());
        case 6: return long2string(pp->getSenderId());
        default: return "";
    }
}

bool OBS_BurstDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    OBS_Burst_Base *pp = (OBS_Burst_Base *)object; (void)pp;
    switch (field) {
        case 1: pp->setNumPackets(string2long(value)); return true;
        case 2: pp->setMinOffset(string2double(value)); return true;
        case 3: pp->setMaxOffset(string2double(value)); return true;
        case 4: pp->setBurstifierId(string2long(value)); return true;
        case 5: pp->setNumSeq(string2long(value)); return true;
        case 6: pp->setSenderId(string2long(value)); return true;
        default: return false;
    }
}

const char *OBS_BurstDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return opp_typename(typeid(cQueue));
        default: return NULL;
    };
}

void *OBS_BurstDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_Burst_Base *pp = (OBS_Burst_Base *)object; (void)pp;
    switch (field) {
        case 0: return (void *)static_cast<cObject *>(&pp->getMessages()); break;
        default: return NULL;
    }
}


