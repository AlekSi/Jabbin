#ifndef XMPP_VCARD_H
#define XMPP_VCARD_H

namespace XMPP {

class VCard
{
public:
	VCard() {}
	// VCard(const VCard &);
	// VCard & operator=(const VCard &);
	// ~VCard();
	//
	// QDomElement toXml(QDomDocument *) const;
	// bool fromXml(const QDomElement &);
	// bool isEmpty() const;
	//
	// const QString &version() const;
	// void setVersion(const QString &);
	//
	QString fullName() const { return QString(); }
	void setFullName(QString) {}
	//
	//
	// const QString &familyName() const;
	// void setFamilyName(const QString &);
	//
	// const QString &givenName() const;
	// void setGivenName(const QString &);
	//
	// const QString &middleName() const;
	// void setMiddleName(const QString &);
	//
	// const QString &prefixName() const;
	// void setPrefixName(const QString &);
	//
	// const QString &suffixName() const;
	// void setSuffixName(const QString &);
	//
	//
	QString nickName() const { return QString(); }
	void setNickName(QString) {}
	//
	//
	// const QByteArray &photo() const;
	// void setPhoto(const QByteArray &);
	// void setPhoto(const QImage &);
	//
	// const QString &photoURI() const;
	// void setPhotoURI(const QString &);
	//
	//
	// const QDate bday() const;
	// void setBday(const QDate &);
	//
	// const QString &bdayStr() const;
	// void setBdayStr(const QString &);
	//
	//
	// class Address {
	// public:
	// 	Address();
	// 	bool isNull() const;
	//
	// 	bool home;
	// 	bool work;
	// 	bool postal;
	// 	bool parcel;
	//
	// 	bool dom;
	// 	bool intl;
	//
	// 	bool pref;
	//
	// 	QString pobox;
	// 	QString extaddr;
	// 	QString street;
	// 	QString locality;
	// 	QString region;
	// 	QString pcode;
	// 	QString country;
	// };
	// typedef QList<Address> AddressList;
	// const AddressList &addressList() const;
	// void setAddressList(const AddressList &);
	//
	// Address address() const;
	// QString locality() const;
	//
	// class Label {
	// public:
	// 	Label();
	//
	// 	bool home;
	// 	bool work;
	// 	bool postal;
	// 	bool parcel;
	//
	// 	bool dom;
	// 	bool intl;
	//
	// 	bool pref;
	//
	// 	QStringList lines;
	// };
	// typedef QList<Label> LabelList;
	// const LabelList &labelList() const;
	// void setLabelList(const LabelList &);
	//
	//
	// class Phone {
	// public:
	// 	Phone();
	//
	// 	bool home;
	// 	bool work;
	// 	bool voice;
	// 	bool fax;
	// 	bool pager;
	// 	bool msg;
	// 	bool cell;
	// 	bool video;
	// 	bool bbs;
	// 	bool modem;
	// 	bool isdn;
	// 	bool pcs;
	// 	bool pref;
	//
	// 	QString number;
	// };
	// typedef QList<Phone> PhoneList;
	// const PhoneList &phoneList() const;
	// void setPhoneList(const PhoneList &);
	//
	//
	// class Email {
	// public:
	// 	Email();
	//
	// 	bool home;
	// 	bool work;
	// 	bool internet;
	// 	bool x400;
	//
	// 	QString userid;
	// };
	// typedef QList<Email> EmailList;
	// const EmailList &emailList() const;
	// void setEmailList(const EmailList &);
	//
	// QString email() const;
	//
	// const QString &jid() const;
	// void setJid(const QString &);
	//
	// const QString &mailer() const;
	// void setMailer(const QString &);
	//
	// const QString &timezone() const;
	// void setTimezone(const QString &);
	//
	//
	// class Geo {
	// public:
	// 	Geo();
	//
	// 	QString lat;
	// 	QString lon;
	// };
	// const Geo &geo() const;
	// void setGeo(const Geo &);
	//
	//
	// const QString &title() const;
	// void setTitle(const QString &);
	//
	// const QString &role() const;
	// void setRole(const QString &);
	//
	//
	// const QByteArray &logo() const;
	// void setLogo(const QByteArray &);
	//
	// const QString &logoURI() const;
	// void setLogoURI(const QString &);
	//
	//
	// const VCard *agent() const;
	// void setAgent(const VCard &);
	//
	// const QString agentURI() const;
	// void setAgentURI(const QString &);
	//
	//
	// class Org {
	// public:
	// 	Org();
	//
	// 	QString name;
	// 	QStringList unit;
	// };
	// const Org &org() const;
	// void setOrg(const Org &);
	//
	//
	// const QStringList &categories() const;
	// void setCategories(const QStringList &);
	//
	// const QString &note() const;
	// void setNote(const QString &);
	//
	// const QString &prodId() const; // it must equal to "Psi" ;-)
	// void setProdId(const QString &);
	//
	// const QString &rev() const;
	// void setRev(const QString &);
	//
	// const QString &sortString() const;
	// void setSortString(const QString &);
	//
	//
	// const QByteArray &sound() const;
	// void setSound(const QByteArray &);
	//
	// const QString &soundURI() const;
	// void setSoundURI(const QString &);
	//
	// const QString &soundPhonetic() const;
	// void setSoundPhonetic(const QString &);
	//
	//
	// const QString &uid() const;
	// void setUid(const QString &);
	//
	QString url() const { return "http://foo.bar/"; }
	// void setUrl(const QString &);
	//
	// const QString &desc() const;
	// void setDesc(const QString &);
	//
	//
	// enum PrivacyClass {
	// 	pcNone = 0,
	// 	pcPublic = 1,
	// 	pcPrivate,
	// 	pcConfidential
	// };
	// const PrivacyClass &privacyClass() const;
	// void setPrivacyClass(const PrivacyClass &);
	//
	//
	// const QByteArray &key() const;
	// void setKey(const QByteArray &);
	//
	// int age() const;
	//
	enum Gender {
		UnknownGender = 0,
		Male,
		Female
	};
	// Gender gender() const;
	// void setGender(Gender);
};

}

#endif
