#if !defined(MMsgRoutingBanUser_H)
#define MMsgRoutingBanUser_H

// MMsgRoutingBanUser.h

#include "msg/TMessage.h"

namespace WONMsg {

class MMsgAuth1KeepAliveHL : public MiniMessage {
public:
    // Default ctor
    MMsgAuth1KeepAliveHL(void);

    // MiniMessage ctor
    explicit MMsgAuth1KeepAliveHL(const MiniMessage& theMsgR);

    // Copy ctor
    MMsgAuth1KeepAliveHL(const MMsgAuth1KeepAliveHL& theMsgR);

    // Destructor
    virtual ~MMsgAuth1KeepAliveHL(void);

    // Assignment
    MMsgAuth1KeepAliveHL& operator=(const MMsgAuth1KeepAliveHL& theMsgR);

    // Virtual Duplicate from MiniMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
    unsigned long GetUserId() const;
    void SetUserId( unsigned long theUserId );

	// Force copy of raw buf if needed.
	// void ForceRawBufOwn();

	// Buffer access
	const unsigned char* GetRawBuf() const;
	unsigned short       GetRawBufLen() const;

    // NOTE: CopyBuf = false currently not implemented
    //
	// Update buffer.  Setting copyBuf to false will cause the specified
	// theBlockP pointer to be stored without copying its contents.  This will
	// improve performance, but theBlockP MUST NOT BE DEALLOCATED while in use
	// by this class.
	void SetRawBuf(const unsigned char* theRawP, unsigned short theLen );

private:
    unsigned long        mUserId;

	WONCommon::RawBuffer mRawBuf;  // Buffer for raw data when owned
};

// Inlines
inline TRawMsg*
MMsgAuth1KeepAliveHL::Duplicate(void) const
{return new MMsgAuth1KeepAliveHL(*this);}

inline unsigned long
MMsgAuth1KeepAliveHL::GetUserId() const
{ return mUserId; }

inline void MMsgAuth1KeepAliveHL::SetUserId(unsigned long theUserId)
{ mUserId = theUserId; }

	// Buffer access
inline const unsigned char*
MMsgAuth1KeepAliveHL::GetRawBuf() const
{ return mRawBuf.data(); }

inline unsigned short
MMsgAuth1KeepAliveHL::GetRawBufLen() const
{ return mRawBuf.size(); }

inline void
MMsgAuth1KeepAliveHL::SetRawBuf(const unsigned char* theRawP, unsigned short theLen )
{ mRawBuf.assign( theRawP, theLen ); }


class MMsgAuth1KeepAliveReplyHL : public MiniMessage {
public:
    // Default ctor
    MMsgAuth1KeepAliveReplyHL(void);

    // MiniMessage ctor
    explicit MMsgAuth1KeepAliveReplyHL(const MiniMessage& theMsgR);

    // Copy ctor
    MMsgAuth1KeepAliveReplyHL(const MMsgAuth1KeepAliveReplyHL& theMsgR);

    // Destructor
    virtual ~MMsgAuth1KeepAliveReplyHL(void);

    // Assignment
    MMsgAuth1KeepAliveReplyHL& operator=(const MMsgAuth1KeepAliveReplyHL& theMsgR);

    // Virtual Duplicate from MiniMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	// Force copy of raw buf if needed.
	// void ForceRawBufOwn();

	// Buffer access
	const unsigned char* GetRawBuf() const;
	unsigned short       GetRawBufLen() const;

    // NOTE: CopyBuf = false currently not implemented
    //
	// Update buffer.  Setting copyBuf to false will cause the specified
	// theBlockP pointer to be stored without copying its contents.  This will
	// improve performance, but theBlockP MUST NOT BE DEALLOCATED while in use
	// by this class.
	void SetRawBuf(const unsigned char* theRawP, unsigned short theLen );

private:
    WONCommon::RawBuffer mRawBuf;
};

// Inlines
inline TRawMsg* MMsgAuth1KeepAliveReplyHL::Duplicate(void) const
    { return new MMsgAuth1KeepAliveReplyHL(*this); }

	// Buffer access
inline const unsigned char*
MMsgAuth1KeepAliveReplyHL::GetRawBuf() const
{ return mRawBuf.data(); }

inline unsigned short
MMsgAuth1KeepAliveReplyHL::GetRawBufLen() const
{ return mRawBuf.size(); }

inline void
MMsgAuth1KeepAliveReplyHL::SetRawBuf(const unsigned char* theRawP, unsigned short theLen )
{ mRawBuf.assign( theRawP, theLen ); }

}; // namespace WONMsg

#endif // MMsgRoutingBanUser_H