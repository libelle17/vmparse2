#include <utime.h>
#include <iomanip>
#ifdef linux
#define vtz '/' // Verzeichnistrennzeichen
#elif __MINGW32__ || _MSC_VER
#include <sys/utime.h>
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
// #include "c:\programme\postgresql\9.1\include\libpq-fe.h"
#include "..\..\..\..\programme\postgresql\9.2\include\libpq-fe.h"
#define vtz '\\' // Verzeichnistrennzeichen
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h> // DIR, dirent, opendir, readdir, PATH_MAX
#include <boost/algorithm/string.hpp>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"
#include "konsole.h"
#include "DB.h"
// #include "iconverter.h"
// #include "datetime_utils.hpp"
#include <regex.h>
#define uchar unsigned char
// G.Schade 22.4.14 für Zusatz-Debugging (SQL): 255, sonst: 0
// #define ZDB 255
#define ZDB 0

const char *logverz; // das reine Verzeichnis
string logpfad; // zusammengesetzt aus Verzeichnis und name
// const char lgs logdatei_a[PATH_MAX+1] ="/var/log/log_vmparse.txt";
// const char *logdatei = logdatei_a;
const char *logdatei; // muss dann auf logpfad zeigen
const char* logdatname;
const char *logdt = "/var/log/vmp.log";
const char* dbq="emails";
const char *lmailadn="lmailad";
const char *lmailsn="lmails";
const char *lmailempfn="lmailempf";
const char *lmailhparn="lmailhpar";
const char *lmailhfeldn="lmailhfeld";
const char *lmailbodyn="lmailbody";
const char *lmailtextn="lmailtext";

const DBSTyp myDBS=MySQL;
// #define mitinhalt

using namespace std;
string out1;
string attverz;
string attanf;

class empftyp 
{
  public:
    string id;
    int empfart;
    void init(){id="";empfart=0;}
    template <typename tC> explicit empftyp (tC vid, int vempfart):id(vid),empfart(vempfart) {
    }
};

class feldtyp 
{
  public:
    int lfdnr;
    string name;
    string wert;
    void init(){lfdnr=0;name="";wert="";}
    template <typename tC> explicit feldtyp (int vlfdnr, tC vname, tC vwert): lfdnr(vlfdnr), name(vname), wert(vwert) {
    }
};

class partyp {
  public:
    int lfdnr;
    int parnr;
    string name;
    string wert;
    void init(){lfdnr=0;parnr=0;name="";wert="";}
    template <typename tC> explicit partyp (int vlfdnr, int vparnr, tC vname, tC vwert): lfdnr(vlfdnr), parnr(vparnr), name(vname), wert(vwert) {
    }
};

class texttyp 
{
  public:
    int lfdnr;
    int art;
    string text;
    void init(){lfdnr=0;art=0;text="";}
    template <typename tC> explicit texttyp (int vlfdnr, int vart, tC vtext): lfdnr(vlfdnr), art(vart), text(vtext) {
    }
};

class bodytyp 
{
  public:
    unsigned lfdnr;
    string CType;
    string charset;
    string name;
    string boundary;
    string type;
    string format;
    string replytype;
    string CTransferEncoding;
    string CDisposition;
    string filename;
    void init(){lfdnr=0;CType="";charset="";boundary="";type="";format="";replytype="";CTransferEncoding="";CDisposition="";filename="";}
    bodytyp(unsigned vlfdnr): lfdnr(vlfdnr){}
    template <typename tC> explicit bodytyp (unsigned vlfdnr, string vCType, string vcharset, string vname, string vboundary, string vtype, string vformat, string vreplytype, string vCTransferEncoding, string vCDisposition, string vfilename): lfdnr(vlfdnr), CType(vCType), charset(vcharset), name(vname), boundary(vboundary), type(vtype), format(vformat), replytype(vreplytype), CTransferEncoding(vCTransferEncoding), CDisposition(vCDisposition), filename(vfilename) {
    }
};

std::ostream& operator<<(std::ostream& os, const vmime::exception& e)
{
  os << " vmime::exceptions::" << e.name() << std::endl;
  os << " what = " << e.what() << std::endl;
  // Recursively print all encapsuled exceptions
  if (e.other() != NULL)
    os << e.other();
  return os;
}
/*
   void showMessage(string nachr) {
   ifstream file;
   file.open(nachr.c_str(), ios::in | ios::binary);
   vmime::utility::inputStreamAdapter is(file );
   vmime::string data;
   vmime::utility::outputStreamStringAdapter os(data);
   vmime::utility::bufferedStreamCopy(is , os);
   vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
   msg->parse(data);
   vmime::messageParser mp(msg);
   int attz=0;
   int ahz = mp.getAttachmentCount();
   for (int i = 0 ; i < ahz ; ++i) {
   vmime::shared_ptr <const vmime::attachment> att = mp.getAttachmentAt(i);           
   vmime::string attData;
   vmime::utility::outputStreamStringAdapter out(attData);
   att->getData()->extract(out);

   const vmime::shared_ptr <const vmime::fileAttachment> fatt = vmime::dynamicCast <const vmime::fileAttachment>(att);
   if (!fatt) {
   "class not convertible\n";
   } else {
   const vmime::fileAttachment::fileInfo &gFI = fatt->getFileInfo();
   if ((&gFI)->hasReadDate()) {
   const vmime::datetime *dt = &(gFI.getReadDate());
   int jahr,mon,tag,std,min,sek,zone;
   dt->getTime(std,min,sek,zone);
   dt->getDate(jahr,mon,tag);
   }
   }
   int fldz = att->getHeader()->getFieldCount();
   for(int i=0;i<fldz;i++) {
   vmime::shared_ptr <const vmime::headerField> aktF= att->getHeader()->getFieldAt(i);
   string name = aktF->getName();
   "Field "<<i<<": "<<name<<endl;
   if (aktF->getValue()!=0) {
   vmime::shared_ptr <const vmime::headerFieldValue> wert = aktF->getValue();
   <<"      "<<wert->generate()<<endl;
   } 
   }

   }
   }
 */
//#define unicode
#ifdef unicode
wstring Utf8ToWstring(const string& inUtf8)
{
  string buf;
  vmime::charset::convert(inUtf8, buf, vmime::charsets::UTF_8, vmime::charsets::ISO8859_1); 

  wstring outUnicode;
  outUnicode.assign((wchar_t*)buf.c_str(), buf.length() / sizeof(wchar_t));
  return outUnicode;
}

// FIX by Elmue:
// Any Codepage to Unicode conversion
#endif
/*
   wstring CpToUTF(const string& in, const vmime::charset& charsetIn)
   {
   string buf;
   vmime::charset::convert(in, buf, charsetIn, vmime::charsets::UTF_8); 

   wstring outUnicode;
   outUnicode.assign((wchar_t*)buf.c_str(), buf.length() / sizeof(wchar_t));
   return outUnicode;
   }
 */
/*
   std::string parameterizedFieldToStructure(vmime::shared_ptr<vmime::parameterizedHeaderField> vmParamField)
   {
   list<string> lParams;
   string buffer;
   vmime::utility::outputStreamStringAdapter os(buffer);

   try {
   vector <vmime::shared_ptr<vmime::parameter> > vParams = vmParamField->getParameterList();
   vector <vmime::shared_ptr<vmime::parameter> >::iterator iParam;

   for (iParam = vParams.begin(); iParam != vParams.end(); iParam++) {
   lParams.push_back("\"" + (*iParam)->getName() + "\"");
   (*iParam)->getValue().generate(os);
   lParams.push_back("\"" + buffer + "\"");
   buffer.clear();
   }
   }
   catch (vmime::exception &e) {
   return "NIL";
   }
   if (lParams.empty())
   return "NIL";
   return "";
// return "(" + boost::algorithm::join(lParams, " ") + ")";
}
 */
void dospeichereAdresse(DB *My, string *Name, string *Mailadr, const char *lmailadn, string *lmailadid, int obempf,/* string *eml,*/ uchar obverb, uchar oblog) 
{
  RS zs(My);
  RS rins(My); // ebenso
  Log(string("")+drot+"  Name: "+schwarz+*Name,obverb,oblog);
  Log(string("")+drot+"  Email: "+schwarz+*Mailadr,obverb,oblog);
  for(int runde=0;runde<2;runde++){
    if (runde==0) zs.Abfrage("SET NAMES 'utf8'");
    else if (runde==1) zs.Abfrage("SET NAMES 'latin1'");
    rins.clear();
    vector<instyp> einf; // fuer alle Datenbankeinfuegungen
    einf.push_back(instyp(My->DBS,"Name",Name));
    einf.push_back(instyp(My->DBS,"Email",Mailadr));
    einf.push_back(instyp(My->DBS,"obEmpf",obempf));
    //    einf.push_back(instyp(My->DBS,"nachr",eml));
    rins.insert(lmailadn,einf, 1,0,ZDB?ZDB:!runde, lmailadid,1);
    if (runde==1) zs.Abfrage("SET NAMES 'utf8'");
    if (*lmailadid!="null") break;
    if (runde==1) {
      Log(string("")+"Fehler "+drot+ltoan(rins.fnr)+schwarz+" bei: "+gelb+rins.sql+schwarz+": "+blau+rins.fehler+schwarz,1,oblog);
      exit(117);
    }
  }
  Log(string("")+drot+string("  lmailad-ID: ")+schwarz+*lmailadid,obverb,oblog);
} // dospeichereAdresse

void speichereSender(DB *My, string *Sender, string *Senderemail, const char *lmailadn, string *lmailadid,/* string *eml,*/ uchar obverb, uchar oblog) 
{
  Log(string("")+drot+"Sender: "+schwarz,obverb,oblog);
  // Sender-Email (lmailad)
  dospeichereAdresse(My,Sender,Senderemail,lmailadn,lmailadid,0,/* eml,*/obverb,oblog);
} // speichereSender

void speichereEmpfaenger(DB *My, boost::shared_ptr<vmime::mailbox> mbox,unsigned empfArt, const char *lmailadn, vector<empftyp> *empfids,/* string *eml,*/ uchar obverb, uchar oblog) 
{
  static vmime::charset ch(vmime::charsets::UTF_8);
  vector<instyp> einf; // fuer alle Datenbankeinfuegungen
  string lmailadid;
  //  vmime::text aN = mbox->getName();
  string aN=mbox->getName().getConvertedText(ch);
  string aEm = mbox->getEmail().toString();
  // Empfaenger (lmailad)
  dospeichereAdresse(My,&aN,&aEm,lmailadn,&lmailadid,1,/*eml,*/obverb,oblog);
  empfids->push_back(empftyp(lmailadid,empfArt));
} // speichereEmpfaenger

int zeigNachricht(string *nachr,struct stat *entrystatp, DB *My, const char* lmailadn, const char *lmailsn, const char *lmailempfn, const char *lmailtextn, const char *lmailbodyn, const char *lmailhfeldn, const char* lmailhparn, regex_t *regexp, unsigned nachrnr, unsigned nachrzahl, uchar keinatt, uchar obverb,uchar oblog) 
{
  Log(violetts+"zeignachricht()"+schwarz+" nachrnr: "+ltoan(nachrnr),obverb,oblog);
  string mailid;
  int attz=0;
  time_t mptime_t=0;
  string Kopfzeile="";
  string Sender="";
  string Senderemail="";
  string attname="";
  string neuname, neupfad;
  unsigned empfz=0;
  unsigned tz=0;
  unsigned ahz=0;
  const uchar mittrans=1;
  vector<instyp> einf; // fuer alle Datenbankeinfuegungen
  vector<empftyp> empfids; // fuer Empfaenger
  vector<texttyp> textids; // fuer Textteile
  vector<bodytyp> bodyids; // fuer Textteile
  vector<feldtyp> hfeldids; // fuer Headerfelder
  vector<partyp> hparids; // fuer Headerparameter
  RS rins(My); // ebenso
  RS zs(My); // Zeichensatz
  string lmailadsid; // ID der aktuellen Mailadresse des Senders
  char letzteaend[20];
  if (nachr->find("Anti Spam Newsletter for November 2003.eml")!=string::npos) {
    Log(string("Datei ")+drot+*nachr+schwarz+" wegen bekannter parse-Fehler von der Bearbeitung ausgeschlossen",1,1);
    return 0; // bleibt bei parse stecken, Stand 23.2.14
  }
  tm *loct = localtime(&entrystatp->st_mtime);
  strftime(letzteaend,sizeof letzteaend, "%d.%m.%y %X", loct);
  Log(string(obverb?"\n":"")+blau+"bearbeite Datei "+"("+string(ltoan(nachrnr+1))+"/"+string(ltoan(nachrzahl))+") ("+gelb+string(ltoan(entrystatp->st_size))+schwarz+" Bytes, "+ blau+letzteaend+schwarz+"):\n"+drot+" "+*nachr+schwarz,1,1,1);  
  //  Log(string(""),1,oblog);
  std::ifstream file;
  stringstream ss(""), ss0("");
  vmime::charset ch(vmime::charsets::UTF_8);
  //  vmime::charset ch(vmime::charsets::ISO8859_15);
  //		file.open("/DATA/Mail/EML/Local Folders/Medizin/BDI/Intensivkurs Angiologie.eml", std::ios::in | std::ios::binary);
  file.open(nachr->c_str(), std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    Log(string("")+rot+"Datei "+blau+*nachr+rot+" nicht zu oeffnen!"+schwarz,1,oblog);
  } else {
    try {
      vmime::utility::inputStreamAdapter is(file );
      vmime::string data;
      vmime::utility::outputStreamStringAdapter os(data);
      // Hier Absturz bei umgekehrten Fragezeichen
      vmime::utility::bufferedStreamCopy(is , os);
      // Actually parse the message
      vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
      //		vmime::shared_ptr <vmime::net::message> nmsg = vmime::make_shared <vmime::net::message>();
      msg->parse(data);
      vmime::shared_ptr <vmime::header> hdr = msg->getHeader();
      int fldz = hdr->getFieldCount();
      for(int i=0;i<fldz;i++) {
        vmime::shared_ptr <vmime::headerField> aktF= hdr->getFieldAt(i);
        string name = aktF->getName();
        Log(string("")+blau+"Header-Feld Nr. "+zustr(i)+": "+schwarz+name,obverb,oblog);
        string ewert="";
        if (aktF->getValue()!=0) {
          //            vmime::shared_ptr <const vmime::headerFieldValue> wert = aktF->getValue();
          ewert=aktF->getValue()->generate();
          Log(string("  ")+gelb+ewert+schwarz,obverb,oblog);
          //          transform(Datei.begin(),Datei.begin()+pos,Datei.begin(),::tolower); // lcase
          string lname = boost::to_lower_copy(name);
          if (lname=="message-id" || lname=="content-id") mailid=ewert;
        } 
        hfeldids.push_back(feldtyp(i,name,ewert));
        //            if (name!="Return-Path" && name!="Delivered-To" && name!="Received" && name!="DKIM-Signature" && name!="DomainKey-Signature" && name!="From" && name!="To" && name!="Reply-To" && name!="Subject" && name!="Message-ID" && name!="List-Unsubscribe" && name!="List-Id" && name!="Mime-Version")  Klammer_auf
        // #if groe 
        //        const vmime::generationContext& ctx = vmime::generationContext::getDefaultContext();
        //        // const vmime::parsingContext& ptx = vmime::parsingContext::getDefaultContext();
        //        size_t groe = aktF->getGeneratedSize(ctx);
        //        Log(string("Groe: ")+gelb+ltoan(groe)+schwarz,obverb,oblog);
        // #endif
#if obbuffer
        const std::vector <vmime::shared_ptr <vmime::component> > children = aktF->getChildComponents();
        for (unsigned j=0; j< children.size();++j){
          Log(string("Buffer: ")+blau+children[j]->generate()+schwarz,obverb,oblog);
        }
#endif
        //        if (name==vmime::fields::CONTENT_TYPE || name==vmime::fields::CONTENT_TRANSFER_ENCODING || name==vmime::fields::CONTENT_DISPOSITION)  Klammer_auf
        if (aktF->getChildComponents().size()>1) {
          vmime::shared_ptr <vmime::parameterizedHeaderField> field = vmime::dynamicCast <vmime::parameterizedHeaderField>(hdr->getFieldAt(i));
          int parz= field->getParameterCount();
          Log(string("  Header-Parz: ")+drot+ltoan(parz)+schwarz,obverb,oblog);
          for(int k=0;k<parz;k++) {
            string PName = field->getParameterAt(k)->getName();
            Log(string("  Header-PName: ")+drot+PName+schwarz,obverb,oblog);
            vmime::word pVal = field->getParameterAt(k)->getValueAs <vmime::word>();
            Log(string("  Header-pVal: ")+drot+pVal.getBuffer()+schwarz,obverb,oblog);
            hparids.push_back(partyp(i,k,PName,pVal.getBuffer()));
          }
          // folgender Fall tritt ein, wenn ein Parameter im aktuellen Fall fehlt, ist also irrelevant
          //        _gKLZ_ else if (name==vmime::fields::CONTENT_TYPE || name==vmime::fields::CONTENT_TRANSFER_ENCODING || name==vmime::fields::CONTENT_DISPOSITION)  Klammer_auf
          //          "Halt!"<<endl;exit(0);
        }

      }

      /*		
            int fldz = hdr->getFieldCount();
            for(int i=0;i<fldz;i++)  Klammer_auf
            vmime::shared_ptr <vmime::headerField> aktF= hdr->getFieldAt(i);
            string name = aktF->getName();
            Log(string("")+blau+"Feld Nr. "+zustr(i)+": "+schwarz+name,1,oblog);
            if (aktF->getValue()!=0)  Klammer_auf
            vmime::shared_ptr <vmime::headerFieldValue> wert = aktF->getValue();
            Log(string("  ")+gelb+wert->generate()+schwarz,1,oblog);
       */		
      vmime::shared_ptr <vmime::body> bdy = msg->getBody();

      //		vmime::shared_ptr <const vmime::bodyPart> par = msg->getParentPart();
      vmime::shared_ptr <vmime::bodyPart> par;
      const vector <vmime::shared_ptr <vmime::bodyPart> > kili = bdy->getPartList();
      Log(string("Body-Kindzahl: ")+gelb+zustr(kili.size())+schwarz,obverb,oblog);
      for (unsigned i = 0 ; i < kili.size() ; ++i)	{
        par = vmime::dynamicCast <vmime::bodyPart>(kili[i]);
        if (par) {
          Log(string("Body-Kind: ")+gelb+zustr(i)+schwarz,obverb,oblog);
          vmime::shared_ptr <vmime::header> hdr = par->getHeader();
          int fldz = hdr->getFieldCount();
          bodytyp body(i);
          for(int i=0;i<fldz;i++) {
            vmime::shared_ptr <vmime::headerField> aktF= hdr->getFieldAt(i);
            string name = aktF->getName();
            Log(string("")+blau+"Body-Feld Nr. "+zustr(i)+": "+schwarz+name,obverb,oblog);
            string ewert="";
            if (aktF->getValue()!=0) {
              //            vmime::shared_ptr <const vmime::headerFieldValue> wert = aktF->getValue();
              ewert=aktF->getValue()->generate();
              Log(string("  ")+gelb+ewert+schwarz,obverb,oblog);
              if (name=="Content-Type")                   body.CType=ewert;
              else if (name=="Content-Transfer-Encoding") body.CTransferEncoding=ewert;
              else if (name=="Content-Disposition")       body.CDisposition=ewert;
            } 
            //            feldids.push_back(feldtyp(i,name,ewert));
            if (aktF->getChildComponents().size()>1) {
              vmime::shared_ptr <vmime::parameterizedHeaderField> field = vmime::dynamicCast <vmime::parameterizedHeaderField>(hdr->getFieldAt(i));
              int parz= field->getParameterCount();
              Log(string("  Body-Parz: ")+drot+ltoan(parz)+schwarz,obverb,oblog);
              for(int k=0;k<parz;k++) {
                string PName = field->getParameterAt(k)->getName();
                Log(string("  Body-PName: ")+drot+PName+schwarz,obverb,oblog);
                string pValS = field->getParameterAt(k)->getValueAs <vmime::word>().getBuffer();
                Log(string("  Body-pVal: ")+drot+pValS+schwarz,obverb,oblog);
                if (PName=="charset") einf.push_back(instyp(My->DBS,"charset",&pValS));
                else if (PName=="name") body.name=pValS;
                else if (PName=="boundary") body.boundary=pValS;
                else if (PName=="type") body.type=pValS;
                else if (PName=="format") body.format=pValS;
                else if (PName=="reply-type") body.replytype=pValS;
                else if (PName=="filename") body.filename=pValS;
              }
            }
          }
          bodyids.push_back(body);
        }
      }

      //		return 0;
      // Now, you can extract some of its components
      Kopfzeile = vmime::dynamicCast <vmime::text>(hdr->Subject()->getValue())->getConvertedText(ch);
      Log(string("") + drot + "Kopfzeile: " + schwarz+ Kopfzeile,obverb,oblog);
      Sender = vmime::dynamicCast <vmime::mailbox>(hdr->From()->getValue())->getName().getConvertedText(ch);
      //      if (Sender==Kopfzeile) Sender="";
      //      Log(string("") + drot + "Sender: " + schwarz + Sender,obverb,oblog);
      Senderemail = vmime::dynamicCast <vmime::mailbox>(hdr->From()->getValue())->getEmail().toString();
      //      Log(string("")+drot+" (Email: "+schwarz+Senderemail+drot+")"+schwarz,obverb,oblog); 
      //        mysql_close(My->conn);
      //        My->init(myDBS,host,"praxis","sonne",dbq,0,0,0,0);
      if (mittrans) mysql_autocommit(My->conn,0);
      //      rins.Abfrage("begin");
      speichereSender(My, &Sender, &Senderemail, lmailadn, &lmailadsid,/* nachr,*/ obverb, oblog);

      vmime::messageParser mp(msg);

      for(unsigned  empfArt=0;empfArt<3;empfArt++) {
        vmime::addressList aL;
        switch (empfArt) {
          case 0: 		aL = mp.getRecipients(); break;
          case 1: 		aL = mp.getCopyRecipients(); break;
          case 2: 		aL = mp.getBlindCopyRecipients(); break;
        }
        string outst = string("")+drot;
        if (!aL.isEmpty())
          switch (empfArt) {
            case 0: 		outst+="Empfaengerzahl: "; break;
            case 1: 		outst+="CC-Empfaengerzahl: "; break;
            case 2: 		outst+="BCC-Empfaengerzahl: "; break;
          }
        //        char numstr[21]; sprintf(numstr,"%ld",aL.getAddressCount());
        string numstr=ltoan(aL.getAddressCount());
        empfz+=aL.getAddressCount();
        for (unsigned int i=0;i<aL.getAddressCount();i++) {
          Log(string("")+drot+"Empfaenger Nr: "+schwarz+zustr(i)+drot+":"+schwarz,obverb,oblog);
          vmime::shared_ptr <vmime::mailbox> mbox = vmime::dynamicCast <vmime::mailbox>(aL.getAddressAt(i));
          if (mbox==NULL){
            Log(string("")+drot+string("mbox leer!")+schwarz,obverb,oblog);
          } else {
            if (mbox->isGroup()){
              vmime::text aN = mbox->getName();
              Log(string("")+drot+"  Gruppenname: "+schwarz+aN.getConvertedText(ch),obverb,oblog);
              vmime::shared_ptr <vmime::mailboxGroup> mboxg = vmime::dynamicCast <vmime::mailboxGroup>(aL.getAddressAt(i));
              int k = mboxg->getMailboxCount();
              Log(string("")+drot+"  Gruppengroesse "+schwarz+zustr(k),obverb,oblog);
              if (k>0) Log("groessegr1",1,oblog); // <<"groessegr1"<<endl;
              for(int kk =0;kk<k;kk++) {
                vmime::shared_ptr <vmime::mailbox> mboxk = vmime::dynamicCast <vmime::mailbox>(mboxg->getMailboxAt(kk));
                Log(string("")+drot+"Vollgruppe!!!!!!!!!!!!!!!!!!!!!"+schwarz,1,oblog);
                speichereEmpfaenger(My, mboxk, empfArt, lmailadn, &empfids,/* nachr,*/ obverb, oblog);
              }
            } else {
              speichereEmpfaenger(My, mbox, empfArt, lmailadn, &empfids,/* nachr,*/ obverb, oblog);
            } // if (mbox->isGroup())
          }
        }  // for (int i=0;i<aL.getAddressCount();i++)
      }// for(int empfArt=0;empfArt<3;empfArt++) 
      //	 vmime::shared_ptr <vmime::datetime> dt = mp.getDate();
      if (mittrans) mysql_commit(My->conn);
      //      rins.Abfrage("commit");
      //        mysql_close(My->conn);
      //        My->init(myDBS,host,"praxis","sonne",dbq,0,0,0,0);
      //      rins.Abfrage("begin");
      tm mptm;
      memset(&mptm, 0, sizeof(struct tm));
#ifdef falschdatum
      const vmime::datetime &dt =	mp.getDate();
      int jahr,mon,tag,std,min,sek,zone;
      dt.getTime(std,min,sek,zone);
      dt.getDate(jahr,mon,tag);
      mptm.tm_sec = sek;
      mptm.tm_min = min;
      mptm.tm_hour= std;
      mptm.tm_mday= tag;
      mptm.tm_mon = mon -1;
      mptm.tm_year= jahr-1900;
      Log(string("")+drot+"Datum: "+schwarz+zustr(tag)+"."+zustr(mon)+"."+zustr(jahr)+" "+zustr(std)+":"+zustr(min)+":"+zustr(sek)+" ("+zustr(zone)+")",obverb,oblog);
#else
      char buf[255];
      char* serg =
        strptime(hdr->Date()->generate().substr(6).c_str(), "%a, %d %b %Y %H:%M:%S %z", &mptm);
      if (!serg) Log(string("")+drot+"Fehler beim Datumsetzen bei: "+blau+hdr->Date()->generate()+schwarz,1,1);
      strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S %z", &mptm);
      Log(string("")+drot+"Datum: "+schwarz+buf,obverb,oblog);
      strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S_%z", &mptm);
      if (mailid=="") mailid = "("+Senderemail+")";
      //      mailid=ersetze(ersetze(mailid,"<","«"),">","»");
      // Kodierung ISO-8859-1
      //     string neuname = ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(string("E×")+buf+" ¯"+mailid+"¯ "+Kopfzeile,"\"","´"),":","¨"),"<","«"),">","»"),"\\","Ì"),"/","Í"),"|","¡"),"*","¹"),"?","¿")," ","·").substr(0,250)+".eml";
      //      wstring neuname = ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(wstring(L"E×")+Utf8ToWstring(buf)+L" ¯"+Utf8ToWstring(mailid)+L"¯ "+Utf8ToWstring(Kopfzeile),L"\"",L"´"),L":",L"¨"),L"<",L"«"),L">",L"»"),L"\\",L"Ì"),L"/",L"Í"),L"|",L"¡"),L"*",L"¹"),L"?",L"¿"),L" ",L"·").substr(0,200)+L".eml";
      //      iconverter utf8_convert("ISO-8859-1","UTF-8");
      //      <<utf8_convert(mailid)<<endl;
      // &#36
      // "«"
      // "»"
      // "¯"
      // "¯ "
      neuname=string("E#")+buf+"#("+mailid+")#"+Kopfzeile.substr(0,150);
      ersetzAlle(&neuname,"\"","´");
      ersetzAlle(&neuname,"'","¯");
      ersetzAlle(&neuname,":","¨");
      ersetzAlle(&neuname,"<","«");
      ersetzAlle(&neuname,">","»");
      ersetzAlle(&neuname,"\\","Ì");
      ersetzAlle(&neuname,"/","Í");
      ersetzAlle(&neuname,"|","¡");
      ersetzAlle(&neuname,"*","¹");
      ersetzAlle(&neuname,"?","¿");
      ersetzAlle(&neuname," ","·");
      neuname=neuname+".eml";
//      neuname = ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(string("E#")+buf+"#("+mailid+")#"+Kopfzeile.substr(0,150),"\"","´"),"'","¯"),":","¨"),"<","("),">",")"),"\\","Ì"),"/","Í"),"|","¡"),"*","¹"),"?","¿")," ","·")+".eml";
      Log(string(" neuname: ")+blau+neuname+schwarz,1,1);
      //      string neupfad  = string("/tmp0/")+neuname;
      string verz = nachr->substr(0,nachr->rfind("/")+1);
      neupfad = verz+neuname;
      string neupfad1=neupfad; // Variante fuer den bash-Befehl
      ersetzAlle(neupfad1,"$","\\$");
      //      system("if ! test -d /tmp0; then mkdir /tmp0; chmod 774 -R /tmp0; chown sturm:praxis -R /tmp0; fi;");
      int obkuerz=0;
      while (rename(nachr->c_str(),neupfad.c_str())){
        obkuerz=1;
        if (neuname.length()<1) {
          cerr<<"Neuname zu stark gekuerzt!"<<endl;
          exit(116);
          }
        neuname=neuname.substr(0,neuname.length()-1);
        neupfad = verz+neuname;
      }
      if (obkuerz) {
        Log(string(" neuname nach Kuerzen: ")+blau+neuname+schwarz,1,1);
      }
      /*
         if (rename(nachr->c_str(),neupfad.c_str())) Klammer_auf
         iconverter c1("UTF-8","CP437");
         iconverter c2("UTF-8","ISO-8859-1");
         iconverter c3("UTF-8","CP1252");
         iconverter c4("ISO-8859-1","CP1252");
         iconverter c5("CP1252","ISO-8859-1");
         iconverter c6("CP1252","UTF-8");
         iconverter c7("CP437","UTF-8");
         iconverter c8("CP437","ISO-8859-1");
         iconverter c9("ISO-8859-1","UTF-8");
         iconverter c10("ISO-8859-1","CP437");
      //<<c1(neupfad.c_str())<<endl;
      //<<c2(neupfad.c_str())<<endl;
      //<<c3(neupfad.c_str())<<endl;
      //<<c4(neupfad.c_str())<<endl;
      <<c5(neupfad.c_str())<<endl;
      //<<c6(neupfad.c_str())<<endl;
      exit(1);
      _gKLZ_
       */
      //      if (rename(nachr->c_str(),neupfad.c_str())) exit(1);
      //      if (system((string("touch \"")+neupfad1+"\"").c_str())) exit(1);
      //        system((string("chmod 774 \"")+neupfad1+"\"").c_str());
      //        system((string("chown sturm:praxis \"")+neupfad1+"\"").c_str());
      struct utimbuf ubuf;
      ubuf.modtime = mktime(&mptm);
      ubuf.actime = ubuf.modtime;
      //        <<"Buf: "<<drot<<buf<<schwarz<<endl;
      if (utime(neupfad.c_str(),&ubuf)) {
        char buf2[30];
        strftime(buf2, sizeof(buf2), "%Y-%m-%d %H:%M:%S %z", &mptm);
        // <<"touch "<<neupfad1<<endl;
        system((string("touch ")+neupfad1+" -d \""+buf2+"\"").c_str());
        //          <<"Fehler: "<<strerror(errno)<<endl;
        //					<<_rot<<"Fehler beim Datumsetzen von '"<<_schwarz<<neuname<<_rot<<"'"<<_schwarz<<endl;
      }
      struct stat Sdatei;
      if (lstat(neupfad.c_str(),&Sdatei)) {
        Log(string("Fehler bei lstat: ")+neupfad,1,1);
        //          exit(0);
      }
      struct tm *Sdtm;
      Sdtm=localtime(&Sdatei.st_mtime);
      // <<"Sdatei.st_mtime: "<<Sdatei.st_mtime<<endl;
      char buf3[30];
      strftime(buf3,sizeof(buf3),"%Y-%m-%d %H:%M:%S %z",Sdtm);
      //        <<"Dateizeit: "<<buf3<<endl;
      if (memcmp(&Sdatei.st_mtime,&ubuf.modtime,sizeof Sdatei.st_mtime)) {
        Log(string("")+rot+"Datum nicht gesetzt bei '"+schwarz+neuname+rot+"'"+schwarz,1,1);
        //          exit(0);
      }
      //        <<"Erfolg!"<<endl;
      //        exit(0);
#endif
      mptime_t=mktime(&mptm);      
      //	 drot<<"Datum: "<<_schwarz<<tag<<"."<<mon<<"."<<jahr<<" "<<std<<":"<<min<<":"<<sek<<" ("<<zone<<")"<<endl;


      /*	
          const int flags = nmsg->getFlags();
          if (flags & vmime::net::message::FLAG_SEEN) <<" - gelesen"<<endl;
          if (flags & vmime::net::message::FLAG_DELETED)
          << " − gelöscht" << std::endl;
       */		

      ahz = mp.getAttachmentCount();
      attz+=ahz;
      Log(string("")+drot+"Die Mail hat: "+schwarz+(!ahz?"keinen":zustr(ahz))+(ahz<2?" Anhang":" Anhänge"),obverb,oblog);
      //		 << drot << "Die Mail hat: "  << _schwarz << ahz << (ahz==1?" Anhang":" Anhänge")  << std::endl;
      for (unsigned i = 0 ; i < ahz ; ++i) {
        vmime::shared_ptr <const vmime::attachment> att = mp.getAttachmentAt(i );			// att;
        Log(string("")+drot+" Anhang Nr: "+schwarz+zustr(i),obverb,oblog);
        //			<< drot << " Anhang Nr: "  << _schwarz << i << endl;
        Log(string("")+drot+" Typ: "+schwarz+att->getType().generate(),obverb,oblog);
        //			<< drot << " Typ: "  << _schwarz << att->getType().generate() << endl;
        Log(string("")+drot+" Encoding: "+schwarz+att->getEncoding().generate(),obverb,oblog);
        //			<< drot << " Encoding: "  << _schwarz << att->getEncoding().generate() << endl;
        attname=att->getName().generate();
        Log(string("")+drot+" Name:  "+schwarz+attname,obverb,oblog);
        //			<< drot << " Name:  "  << _schwarz << att->getName().generate() << endl;
        Log(string("")+drot+" Beschreibung:  "+schwarz+att->getDescription().generate(),obverb,oblog);
        //			<< drot << " Beschreibung:  "  << _schwarz << att->getDescription().generate() << endl;
        //			vmime::shared_ptr <const vmime::header> hdr = att->getHeader();
        //			vmime::shared_ptr <const vmime::headerField> datum = hdr->Date();
        string s_Data;
        vmime::utility::outputStreamStringAdapter i_Stream(s_Data);
        att->getData()->extract(i_Stream);
        Log(string(" ")+drot+"Attachment-Groesse: "+schwarz+ltoan(s_Data.length()),obverb,oblog); 
#ifdef ausgabe1
        ofstream attdatei;
        attdatei.open(out1+attname).c_str());
        attdatei<<s_Data;
        attdatei.close();
#endif
        //        exit(0);

        //			const vmime::fileAttachment fatt = mp.getAttachmentAt(i);

        // das folgende kommt nicht in die Schleife bei if (children.size()>1), die Werte stehen aber alle bei Body
#if attparameter 
        int fldz = att->getHeader()->getFieldCount();
        Log(string("")+blau+"Attachmentheaderfeldzahl: "+schwarz+zustr(fldz),obverb,oblog);
        for(int i=0;i<fldz;i++) {
          vmime::shared_ptr <const vmime::headerField> aktF= att->getHeader()->getFieldAt(i);
          string name = aktF->getName();
          Log(string("")+blau+"Attachment-Feld Nr. "+zustr(i)+": "+schwarz+name,obverb,oblog);
          string ewert="";
          if (aktF->getValue()!=0) {
            //            vmime::shared_ptr <const vmime::headerFieldValue> wert = aktF->getValue();
            ewert=aktF->getValue()->generate();
            Log(string("  ")+gelb+ewert+schwarz,obverb,oblog);
          } 
          feldids.push_back(feldtyp(i,name,ewert));

          //        const std::vector <vmime::shared_ptr <const vmime::component> > children = aktF->getChildComponents();
          // aus dem Quellcode von headerFields::getChildComponents; die Originalfunktion kann mangels const nicht genommen werden
          std::vector <vmime::shared_ptr <const vmime::component> > children;
          if (aktF->getValue()) {
            children.push_back(aktF->getValue());
          }
          if (children.size()>1) {
            vmime::shared_ptr <vmime::parameterizedHeaderField> field = vmime::dynamicCast <vmime::parameterizedHeaderField>(hdr->getFieldAt(i));
            int parz= field->getParameterCount();
            Log(string("Att-Parz: ")+drot+ltoan(parz)+schwarz,obverb,oblog);
            for(int k=0;k<parz;k++) {
              string PName = field->getParameterAt(k)->getName();
              Log(string("Att-PName: ")+drot+PName+schwarz,obverb,oblog);
              vmime::word pVal = field->getParameterAt(k)->getValueAs <vmime::word>();
              Log(string("Att-pVal: ")+drot+pVal.getBuffer()+schwarz,obverb,oblog);
            }
          }
        } // for(int i=0;i<fldz;i++)
#endif

        /*
           stringstream ssdt("");
           ssdt << vmime::dynamicCast <vmime::text>(hdr->Date()->getValue())->getConvertedText(ch) ;
         */
        vmime::string attData;
        vmime::utility::outputStreamStringAdapter out(attData);
        att->getData()->extract(out);
        if (!keinatt) {
          int reti=regexec(regexp, attname.c_str(),0,NULL,0);
          if (reti) Log(string(rot)+"'"+attname+schwarz+"' passt nicht ins Muster",obverb,oblog);
          if (attname=="") Log(string(rot)+"'"+attname+schwarz+" =Name des Attachments, wird nicht gespreichert ",obverb,oblog);
          if (!reti) if (attname!="") { // (attname==attmuster)  Klammer_auf
            ofstream attdatei;
            strftime(buf, sizeof(buf), "%d.%m.%Y %H.%M.%S", &mptm);
            ss.str("");
            ss<<ersetzefuerdatei(attverz)<<ersetzefuerdatei(Kopfzeile)<<" - "<<buf<<" - "<<attname.c_str();
            Log(string("Schreibe Attachm'datei: ")+blau+ss.str()+schwarz,1,1);
            //            ss<<attverz<<"/"<<Kopfzeile<<" "<<mptime_t<<".pdf";
            system(string(("chmod 774 \"")+ss.str()+"\"").c_str());
            system(string(("chown schade:praxis \"")+ss.str()+"\"").c_str());
            attdatei.open(ss.str().c_str());
            attdatei<<s_Data;
            attdatei.close();
            struct utimbuf ubuf;
            strftime(buf, sizeof(buf), "%d.%m.%Y %H.%M.%S %z", &mptm);
            Log(string("Dateizeit: ")+buf,obverb,oblog); // asctime(&mptm)
            ubuf.modtime = mptime_t ;
            ubuf.actime = mptime_t ;
            // "mptime_t: "<<mptime_t<<endl;
            if (utime(ss.str().c_str(),&ubuf)) {
              Log(string("")+rot+"Fehler beim Datumsetzen von '"+ss.str()+rot+"'"+schwarz,1,oblog);
              //								   		<<_rot<<"Fehler beim Datumsetzen von '"<<_schwarz<<ausgabe<<_rot<<"'"<<_schwarz<<endl;
            }
          } // (attname!="")
        } // (!keinatt)
#ifdef mitattinhalt
        Log(string("")+drot+" Daten: "+schwarz+attData,obverb,oblog);
#endif			
#ifdef ausgabe0
        //			<< drot << " Daten:  "  << _schwarz << attData <<endl; // att->getData() << endl;
        ofstream ofile;
        ss.str("");
        ss0.str("");
        ss0<<attname;
        bool obdatei = (*(ss0.str().c_str()));
        if (obdatei) {
          // wenn ausgabe0 definiert wird, muss nachr im Namen durch etwas anderes ersetzt werden, z.B. E#+Datum+ContontID
          ss<<attanf<<"_"<<ersetze(nachr->c_str(),"/","_")<<"_"<<++attz<<"_"<<sersetze(sersetze(att->getName().getConvertedText(ch),"?","_Fgz_"),"*","_Stern_");
          Log(string("")+drot+"Attachment-Namen: "+schwarz+ss.str(),obverb,oblog);
          //			<<drot<<"Attachment-Namen: "<<_schwarz<<ss.str()<<endl;
          ofile.open(ss.str().c_str(), std::ios::out | std::ios::binary);
          ofile<<attData;
          ofile.close();
          struct utimbuf ubuf;
          time_t modz;
          struct tm tm0;
          tm0.tm_year = jahr-1900;
          tm0.tm_mon = mon-1;
          tm0.tm_mday = tag;
          //          Log("Stunde: "+zustr(std),1,oblog);
          tm0.tm_hour = std;
          tm0.tm_min = min;
          tm0.tm_sec = sek;
          //																				tm0.tm_isdst=1;
          tm0.tm_isdst=-1; // sonst wird zufällig ab und zu eine Stunde abgezogen
          modz=mktime(&tm0);            
          Log(string("Dateizeit: ")+asctime(&tm0),obverb,oblog);
          ubuf.modtime = modz ;
          ubuf.actime = modz ;
          if (utime(ss.str().c_str(),&ubuf)) {
            Log(string("")+rot+"Fehler beim Datumsetzen von '"+ss.str()+rot+"'"+schwarz,1,oblog);
            //								   		<<_rot<<"Fehler beim Datumsetzen von '"<<_schwarz<<ausgabe<<_rot<<"'"<<_schwarz<<endl;
          }
        } // if (obdatei)
        // das folgende tritt nie ein
#endif
#ifdef nie
        if (obdatei) {
          try {
            Log("Vor Dateianalyse",obverb,oblog);
            const vmime::shared_ptr <const vmime::fileAttachment> fatt = vmime::dynamicCast <const vmime::fileAttachment>(att);
            if (!fatt) {
              Log("kein Fileattachment",obverb,oblog);
            } else {
              const vmime::fileAttachment::fileInfo &gFI = fatt->getFileInfo();
              if ((&gFI)->hasReadDate()) {
                const vmime::datetime *dt = &(gFI.getReadDate());
                int jahr,mon,tag,std,min,sek,zone;
                dt->getTime(std,min,sek,zone);
                dt->getDate(jahr,mon,tag);
                Log(string("")+drot+"Aenderungsdatum: "+schwarz+zustr(tag)+"."+zustr(mon)+"."+zustr(jahr)+" "+zustr(std)+":"+zustr(sek)+" ("+zustr(zone)+")",obverb,oblog);
              }
            }
          }
          catch (vmime::exception& e)	{
            cerr<<e;
            Log(string("")+drot+"vmime::exception: "+schwarz+e.what(),1,oblog);
          }
          catch (std::exception& e) {
            std::cerr<<e.what();
          }
        } // if (obdatei)
#endif
      } // for (int i = 0 ; i < ahz ; ++i)

      tz = mp.getTextPartCount();
      Log(string("")+drot+"Die Mail hat: "+schwarz+zustr(tz)+drot+" Textteile."+schwarz,obverb,oblog);
      string outs;
      string *textoutsp;
      for (unsigned i=0; i<tz;++i) {
        vmime::shared_ptr <const vmime::textPart> tp = mp.getTextPartAt(i );
        vmime::utility::outputStreamStringAdapter out(outs);
        //      vmime::utility::charsetFilteredOutputStream fout(tp->getCharset(), vmime::charset ("utf−8"), out);		 
        int oz;
#ifdef mitinhalt		 
        Log(string("")+drot+"Textteil "+schwarz+zustr(i)+drot+":"+schwarz,obverb,oblog);
#endif		
        if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_HTML) {
          vmime::shared_ptr <const vmime::htmlTextPart> htp = vmime::dynamicCast <const vmime::htmlTextPart>(tp);
          htp->getPlainText()->extract(out);
          oz = htp->getObjectCount();
          for (int j = 0; j < oz; ++j) {
            vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> obj = htp->getObjectAt(j);
            Log(string("")+drot+"Objekt-ID: "+schwarz+obj->getId(),obverb,oblog);
            stringstream oDstrstr;
            oDstrstr<<obj->getData();
            Log(string("")+drot+"Objekt-Daten: "+schwarz+oDstrstr.str(),obverb,oblog);
//            textids.push_back(texttyp(atol(obj->getId().c_str()),0,oDstrstr.str())); // Zeiger hexadezimal
            if (outs!=*textoutsp) {
             cout<<violett<<outs.length()<<schwarz<<endl;
            textids.push_back(texttyp(atol(obj->getId().c_str()),0,outs.c_str()));
            }
          }
#ifdef mitinhalt		 
          Log(string("")+rot+"HTML-Text: "+schwarz+outs,obverb,oblog);
#endif		
        } else {
          tp->getText()->extract(out);
          textids.push_back(texttyp(0,1,outs.c_str()));
          textoutsp=&outs;
#ifdef mitinhalt		 
          Log(string("")+drot+"Text: "+schwarz+outs,obverb,oblog);
#endif		
        } // if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_HTML)  
#ifdef mitinhalt		 
        //        CpToWstring(outs, htp.getCharset());
#endif		

      } // for (int i=0; i<tz;++i)
    } // try
    // VMime exception
    catch (vmime::exception& e)	{
      Log(string("")+drot+"vmime::exception: "+schwarz+e.what(),1,oblog);
      //		throw;
    } // catch (vmime::exception& e)
    // Standard exception
    catch (std::exception& e)	{
      Log(string("")+drot+"std::exception: "+schwarz+e.what(),1,oblog);
      //throw;
    }

    //	Log("\n",1,oblog);
    //	<< std::endl;
  }

  // Email (lmails)
  string lmailid;
  for(int runde=0;runde<2;runde++){
    if (runde==0) zs.Abfrage("SET NAMES 'utf8'");
    else if (runde==1) zs.Abfrage("SET NAMES 'latin1'");
    rins.clear();
    vector<instyp> einf; // fuer alle Datenbankeinfuegungen
    einf.push_back(instyp(My->DBS,"EML",&neupfad));
    einf.push_back(instyp(My->DBS,"EMLu",nachr));
    einf.push_back(instyp(My->DBS,"Datum",&mptime_t));
    einf.push_back(instyp(My->DBS,"Dateidatum",&entrystatp->st_mtime));
    einf.push_back(instyp(My->DBS,"Dateizeit",entrystatp->st_mtime));
    einf.push_back(instyp(My->DBS,"Kopfzeile",&Kopfzeile));
    einf.push_back(instyp(My->DBS,"SenderID",&lmailadsid));
    //  einf.push_back(instyp(My->DBS,(char*)"Sendername",(char*)Sender.c_str()));
    //  einf.push_back(instyp(My->DBS,(char*)"Senderemail",(char*)Senderemail.c_str()));
    einf.push_back(instyp(My->DBS,"Empfaengerzahl",empfz));
    einf.push_back(instyp(My->DBS,"Anhangzahl",ahz));
    //  Log(string("vor Speicherung von lmails, Runde: ")+ltoan(runde),1,1);
    //#undef ZDB
    //#define ZDB 255
    rins.insert(lmailsn,einf, 1,0,ZDB?ZDB:!runde,&lmailid,1); 
    //#undef ZDB
    //#define ZDB 0
    if (runde==1) zs.Abfrage("SET NAMES 'utf8'");
    if (lmailid!="null") break;
    if (runde==1) {
      Log(string("")+"Fehler "+drot+ltoan(rins.fnr)+schwarz+" bei: "+gelb+rins.sql+schwarz+": "+blau+rins.fehler+schwarz,1,oblog);
      exit(127);
    }
  }

  if (mittrans) mysql_commit(My->conn);
  //    rins.Abfrage("commit");
  //        mysql_close(My->conn);
  //        My->init(myDBS,host,"praxis","sonne",dbq,0,0,0,0);
  //    rins.Abfrage("begin");
  // Verknuepfung Email-Empfaenger (lmailempfn)
  for(size_t i=0;i<empfids.size();++i){
    rins.clear();
    vector<instyp> einf; // fuer alle Datenbankeinfuegungen
    einf.push_back(instyp(My->DBS,"EmpfArt",empfids[i].empfart));
    einf.push_back(instyp(My->DBS,"lmailadID",&empfids[i].id));
    einf.push_back(instyp(My->DBS,"lmailsID",&lmailid));
    /*          Log(string("Empfart: ")+ltoan(empfids[i].empfart),obverb,oblog);
                Log(string("lmailadID: ")+empfids[i].id,obverb,oblog);
                Log(string("lmailsID: ")+lmailid,obverb,oblog); */
    rins.insert(lmailempfn,einf,!i,i<empfids.size()-1,ZDB); // nur Zahlen, daher keine Zeichensatzprobleme zu erwarten
    //      rins.insert(lmailempfn,einf,1,0); // nur Zahlen, daher keine Zeichensatzprobleme zu erwarten
  }

  // Header-Felder
  for(int runde=0;runde<2;runde++){
    if (runde==0) zs.Abfrage("SET NAMES 'utf8'");
    else if (runde==1) zs.Abfrage("SET NAMES 'latin1'");
    string lmailhfeldid; // ID des aktuellen Textteiles
    for(size_t i=0;i<hfeldids.size();i++){
      RS zs(My);
      rins.clear();
      vector<instyp> einf; // fuer alle Datenbankeinfuegungen
      einf.push_back(instyp(My->DBS,"lmailsID",&lmailid));
      einf.push_back(instyp(My->DBS,"lfdnr",hfeldids[i].lfdnr));
      einf.push_back(instyp(My->DBS,"name",&hfeldids[i].name));
      einf.push_back(instyp(My->DBS,"wert",&hfeldids[i].wert));
      rins.insert(lmailhfeldn,einf,!i,i<hfeldids.size()-1,ZDB?ZDB:!runde,&lmailhfeldid,1);
      // rins.insert(lmailhfeldn,einf,!i,i<hfeldids.size()-1,0,&lmailhfeldid,1);
    }
    if (runde==1) zs.Abfrage("SET NAMES 'utf8'");
    if (lmailhfeldid!="null") break;
    if (runde==1) {
      Log(string("")+"Fehler "+drot+ltoan(rins.fnr)+schwarz+" bei: "+gelb+rins.sql+schwarz+": "+blau+rins.fehler+schwarz,1,oblog);
      exit(126);
    }
  }

  // Header-Parameter
  for(int runde=0;runde<2;runde++){
    if (runde==0) zs.Abfrage("SET NAMES 'utf8'");
    else if (runde==1) zs.Abfrage("SET NAMES 'latin1'");
    string lmailhparid; // ID des aktuellen Textteiles
    for(size_t i=0;i<hparids.size();i++){
      RS zs(My);
      rins.clear();
      vector<instyp> einf; // fuer alle Datenbankeinfuegungen
      einf.push_back(instyp(My->DBS,"lmailsID",&lmailid));
      einf.push_back(instyp(My->DBS,"lfdnr",hparids[i].lfdnr));
      einf.push_back(instyp(My->DBS,"parnr",hparids[i].parnr));
      einf.push_back(instyp(My->DBS,"name",&hparids[i].name));
      einf.push_back(instyp(My->DBS,"wert",&hparids[i].wert));
      rins.insert(lmailhparn,einf,!i,i<hparids.size()-1,ZDB?ZDB:!runde,&lmailhparid,1);
    }
    if (runde==1) zs.Abfrage("SET NAMES 'utf8'");
    if (lmailhparid!="null") break;
    if (runde==1) {
      Log(string("")+"Fehler "+drot+ltoan(rins.fnr)+schwarz+" bei: "+gelb+rins.sql+schwarz+": "+blau+rins.fehler+schwarz,1,oblog);
      exit(125);
    }
  }

  // Body-Teile der Mail (lmailbody)
  for(size_t i=0;i<bodyids.size();++i){
    //    size_t begin = 0;
    //    for(size_t i=begin;i<bodyids.size();++i) Klammer_auf
    string lmailbodyid; // ID des aktuellen Textteiles
    RS zs(My);
    for(int runde=0;runde<2;runde++){
      if (runde==0) zs.Abfrage("SET NAMES 'utf8'");
      else if (runde==1) zs.Abfrage("SET NAMES 'latin1'");
      rins.clear();
      vector<instyp> einf; // fuer alle Datenbankeinfuegungen
      einf.push_back(instyp(My->DBS,"lmailsID",&lmailid));
      einf.push_back(instyp(My->DBS,"lfdnr",bodyids[i].lfdnr));
      einf.push_back(instyp(My->DBS,"CType",&bodyids[i].CType));
      einf.push_back(instyp(My->DBS,"CTransfer-Encoding",&bodyids[i].CTransferEncoding));
      einf.push_back(instyp(My->DBS,"CDisposition",&bodyids[i].CDisposition));
      einf.push_back(instyp(My->DBS,"name",&bodyids[i].name));
      einf.push_back(instyp(My->DBS,"boundary",&bodyids[i].boundary));
      einf.push_back(instyp(My->DBS,"type",&bodyids[i].type));
      einf.push_back(instyp(My->DBS,"format",&bodyids[i].format));
      einf.push_back(instyp(My->DBS,"reply-type",&bodyids[i].replytype));
      einf.push_back(instyp(My->DBS,"filename",&bodyids[i].filename));
      // <<"beginne mit i: "<<i<<" bodyids.size(): "<<bodyids.size()<<" runde: "<<runde<<endl;
      //        rins.insert(lmailbodyn,einf,i==begin,i<bodyids.size()-1,!runde,&lmailbodyid,1);
      // sammelinsert fuehrt bei mehreren Mails, z.B. /DATA/Mail/EML/lrz/new/1362062408.M747395P4980Q591R7f09d9db3351c3f9.linux
      // zum Segmentation fault in mysql_query bei doAbfrage bei select column_... and extra = ...,
      // wenn vorher der Datensatz 4 (von 0-5) mit dabei ist
      rins.insert(lmailbodyn,einf,1,0,ZDB?ZDB:!runde,&lmailbodyid,1);
      // <<"fertig mit i: "<<i<<" bodyids.size(): "<<bodyids.size()<<" runde: "<<runde<<endl;
      if (runde==1) zs.Abfrage("SET NAMES 'utf8'");
      if (lmailbodyid!="null") break;
      if (runde==1) {
        Log(string("")+"Fehler "+drot+ltoan(rins.fnr)+schwarz+" bei: "+gelb+rins.sql+schwarz+": "+blau+rins.fehler+schwarz,1,oblog);
        exit(124);
      }
    }
  }
//  int aufhoern=0;

  // Text-Teile der Mail (lmailtext)
  for(int runde=0;runde<2;runde++){
    if (runde==0) zs.Abfrage("SET NAMES 'utf8'");
    else if (runde==1) zs.Abfrage("SET NAMES 'latin1'");
    string lmailtextid; // ID des aktuellen Textteiles
    for(size_t i=0;i<textids.size();++i){
      RS zs(My);
      rins.clear();
      vector<instyp> einf; // fuer alle Datenbankeinfuegungen
      einf.push_back(instyp(My->DBS,"lmailsID",&lmailid));
      einf.push_back(instyp(My->DBS,"lfdnr",textids[i].lfdnr));
      einf.push_back(instyp(My->DBS,"art",textids[i].art));
      einf.push_back(instyp(My->DBS,"text",&textids[i].text));
      //    #undef ZDB
      //    #define ZDB 255
//      rins.insert(lmailtextn,einf,!i,i<textids.size()-1,0,&lmailtextid,1);
      rins.insert(lmailtextn,einf,!i,i<textids.size()-1,ZDB?ZDB:(runde?0:2),&lmailtextid,1);
      // if (textids[i].art==0) aufhoern=1;
      //    #undef ZDB
      //    #define ZDB 0
    }
    if (runde==1) zs.Abfrage("SET NAMES 'utf8'");
    if (lmailtextid!="null") break;
    if (runde==1) {
      Log(string("")+"Fehler "+drot+ltoan(rins.fnr)+schwarz+" bei: "+gelb+rins.sql+schwarz+": "+blau+rins.fehler+schwarz,1,oblog);
      cout<<"Tabelle: "<<lmailtextn<<endl;
      cout<<"Textlaenge: "<<textids[textids.size()-1].text.length()<<endl;
      cout<<"Fehler: "<<rins.fnr<<endl;
      exit(122);
    }
  }

  if (mittrans) mysql_commit(My->conn);
  //passager    RS unl(My,"unlock tables");
  //        mysql_close(My->conn);
  //        My->init(myDBS,host,"praxis","sonne",dbq,0,0,0,0);
  //    rins.Abfrage("commit");
  if (mittrans) mysql_autocommit(My->conn,1);
 //     if (aufhoern) exit(123);
  return attz;
} // zeigNachricht

void loescherest(DB* My, short oblog,stringstream *ausgabep)
{
  RS ueb0(My,string("select id from ")+lmailsn,ZDB);
  *ausgabep<<", "<<drot<<My->affrows()<<schwarz<<" Saetze noch uebrig.";
  Log(ausgabep->str(),1,oblog);

  RS loe3(My,string("delete from t using ")+lmailtextn+" t left join "+lmailsn+" m on t.lmailsid = m.id where isnull(m.id)",ZDB);
  ausgabep->str("");
  *ausgabep<<drot<<My->affrows()<<schwarz<<" Eintraege aus "<<blau<<lmailtextn<<schwarz<<" geloescht";
  RS ueb3(My,string("select id from ")+lmailtextn,ZDB);
  *ausgabep<<", "<<drot<<My->affrows()<<schwarz<<" Saetze noch uebrig.";
  Log(ausgabep->str(),1,oblog);

  RS loe1(My,string("delete from e using ")+lmailempfn+" e left join "+lmailsn+" m on e.lmailsid = m.id where isnull(m.id)",ZDB);
  ausgabep->str("");
  *ausgabep<<drot<<My->affrows()<<schwarz<<" Eintraege aus "<<blau<<lmailempfn<<schwarz<<" geloescht";
  RS ueb1(My,string("select id from ")+lmailempfn,ZDB);
  *ausgabep<<", "<<drot<<My->affrows()<<schwarz<<" Saetze noch uebrig.";
  Log(ausgabep->str(),1,oblog);

  RS loe4(My,string("delete from e using ")+lmailhfeldn+" e left join "+lmailsn+" m on e.lmailsid = m.id where isnull(m.id)",ZDB);
  ausgabep->str("");
  *ausgabep<<drot<<My->affrows()<<schwarz<<" Eintraege aus "<<blau<<lmailhfeldn<<schwarz<<" geloescht";
  RS ueb4(My,string("select id from ")+lmailhfeldn,ZDB);
  *ausgabep<<", "<<drot<<My->affrows()<<schwarz<<" Saetze noch uebrig.";
  Log(ausgabep->str(),1,oblog);

  RS loe5(My,string("delete from e using ")+lmailhparn+" e left join "+lmailsn+" m on e.lmailsid = m.id where isnull(m.id)",ZDB);
  ausgabep->str("");
  *ausgabep<<drot<<My->affrows()<<schwarz<<" Eintraege aus "<<blau<<lmailhparn<<schwarz<<" geloescht";
  RS ueb5(My,string("select id from ")+lmailhparn,ZDB);
  *ausgabep<<", "<<drot<<My->affrows()<<schwarz<<" Saetze noch uebrig.";
  Log(ausgabep->str(),1,oblog);

  RS loe2(My,string("delete from a using ")+lmailadn+" a where (select count(0) from "+lmailempfn+" e where e.lmailadid=a.id)=0  and (select count(0) from "+lmailsn+" m where m.senderid=a.id)=0",ZDB);
  ausgabep->str("");
  *ausgabep<<drot<<My->affrows()<<schwarz<<" Eintraege aus "<<blau<<lmailadn<<schwarz<<" geloescht";
  RS ueb2(My,string("select id from ")+lmailadn,ZDB);
  *ausgabep<<", "<<drot<<My->affrows()<<schwarz<<" Saetze noch uebrig.";
  Log(ausgabep->str(),1,oblog);
}

void loescheab(DB* My, string *mailpfadp, string *abzp, uchar obverb, short oblog)
{
  stringstream ausgabe;
  string lmails_sql;
  lmails_sql=string("delete from ")+lmailsn+" where EML like \""+*mailpfadp+"%\""+" and datum >= "+*abzp;
  RS loe0(My,lmails_sql,255);
  ausgabe<<drot<<(loe0.obfehl?0:My->affrows())<<schwarz<<" Eintraege aus "<<blau<<lmailsn<<schwarz<<" geloescht";
  loescherest(My,oblog,&ausgabe);
}

void zeigstandan(DB* My,short oblog) 
{
  stringstream ausgabe;
  const char *tabl[]={lmailsn,lmailadn,lmailbodyn,lmailempfn,lmailhfeldn,lmailhparn,lmailtextn};
  for(unsigned iz=0;iz<sizeof tabl/sizeof *tabl;iz++) {
    RS ueb0(My,string("select id from ")+tabl[iz],ZDB);
    ausgabe<<drot<<My->affrows()<<schwarz<<" Saetze in: "<<blau<<tabl[iz]<<schwarz;
    if (iz<sizeof tabl/sizeof *tabl-1) ausgabe<<endl;
  }
  Log(ausgabe.str(),1,oblog);
}

void loeschepfad(DB* My, string *pfad, unsigned long minuten, short oblog, uchar obincon=0) 
{
  stringstream ausgabe;
  Log(drot+(obincon?"2 ":string(ltoan(minuten))+" Minuten alte, ")+string("vorher dort eingetragene Dateien werden geloescht!"));
  //    RS loe1(My,string("delete from lmailad where id in (select distinct a.id from lmails m left join lmailempf e on e.lmailsid = m.id left join lmailad a on a.id = e.lmailadid left join lmailempf e2 on e2.lmailadid = a.id left join lmails m2 on m2.id = e2.lmailsid and m2.eml not like \"")+*pfad+"%\" where m.eml like \""+*pfad+"%\")");
  //    Log(string("")+drot+ltoan(My->affrows())+schwarz+" Eintraege aus "+blau+lmailadn+schwarz+" geloescht",1,oblog);
  string lmails_sql;
  if (obincon) {
    lmails_sql=string("delete from ")+lmailsn+" order by id desc limit 2";
  } else {
    lmails_sql=string("delete from ")+lmailsn+" where EML like \""+*pfad+"%\""+(minuten?string(" and timestampdiff(minute,dateidatum,now())<=")+ltoan(minuten):"");
  }
  RS loe0(My,lmails_sql,ZDB);
  ausgabe<<drot<<(loe0.obfehl?0:My->affrows())<<schwarz<<" Eintraege aus "<<blau<<lmailsn<<schwarz<<" geloescht";
  loescherest(My,oblog,&ausgabe);
}

void loescheundzeigean(uchar dodel,DB* My,string *mailpfad,unsigned long minuten, uchar incon, uchar zeigstand, short oblog)
{
  // Inkonsistenz nach moeglichem Programmabsturz beseitigen
  if (dodel){
    loeschepfad(My, mailpfad, minuten, oblog);
  } else if (incon) {
    loeschepfad(My, mailpfad, minuten, oblog,1);
  } else if (zeigstand) {
    zeigstandan(My,oblog);
  }
}

#define ZPKT(mldg) tende=std::clock(); mldg<<rot<<setprecision(7)<<fixed<<((tende-tstart)/CLOCKS_PER_SEC)<<schwarz<<setprecision(0)<<" s\n";tstart=std::clock();

void testepfad(string *pfad, uchar *wiepfadp, unsigned long *minuten, uchar oblog) 
{
  // wiepfad 0: nichts, 1: Datei, 2: Verzeichnis
  struct stat entrystat;
  int Dateityp;
  if (lstat(pfad->c_str() , &entrystat)) {
    Log(string("lstat fehlgeschlagen: ") + strerror(errno) + " bei Verzeichnis/Datei:"+ *pfad,1,1,1);
    *wiepfadp=0;
  } else { 
    Dateityp = entrystat.st_mode & S_IFMT;
    switch (Dateityp) {
      case S_IFDIR:    // Verzeichnis
        Log(string("Es werden ")+(*minuten?string("bis zu ")+drot+ltoan(*minuten)+schwarz+" Minuten":string(drot)+"beliebig"+schwarz)+" alte Mails beruecksichtigt.",1,oblog);
        *wiepfadp=2;
        break;
      case S_IFREG:   // Datei
        *minuten=0;
        Log(string("Da ein Dateiname uebergeben wurde, wird keine Altergrenze zur Bearbeitung beruecksichtigt."),1,oblog);
        *wiepfadp=1;
        break;
    }
  } 
}

int beaVerz(string *pfad,uchar wiepfad, unsigned long minuten,DB *My, const char* lmailadn, const char* lmailsn, const char* lmailempfn, const char *lmailtextn, const char *lmailbodyn, const char* lmailhfeldn, const char *lmailhparn, regex_t *regexp, unsigned long* geszahl, unsigned long* neuzahl, uchar obopendir=0, short logscreen=0, uchar keinatt=0, uchar obverb=0,short oblog=0) // logscreen,oblog)
{
  struct stat entrystat;
  static int attz=0;
  // double tstart = std::clock(), tende;
  vector<string> verg;

  if (wiepfad==1) {
    verg.push_back(*pfad);
    Log(string("")+drot+"Dateien aus find: "+schwarz+"1",1,oblog);
  } else if (!obopendir) {
    Log(string(rot)+"Arbeite mit find."+schwarz,obverb,oblog);
    // ZPKT( "vor find: ")
    string cmd=string("find \"")+*pfad+"\" -type f"+(minuten?string(" -mmin -")+ltoan(minuten):"");
    // ZPKT(" nach find: ")
    Log(string("")+drot+"Suchbefehl: "+schwarz+cmd,1,oblog);
    //  if (obverb) system(cmd.c_str());
    if (FILE* pipe = popen(cmd.c_str(), "r")){
      while(!feof(pipe)) {
        char buffer[1280];
        if(fgets(buffer, sizeof buffer, pipe) != NULL){
          size_t posi;
          if (buffer[posi=strlen(buffer)-1]==10) buffer[posi]=0;
          verg.push_back(string(buffer));
        }
      }
      pclose(pipe);
    }
    // ZPKT(" nach push: ")
    Log(string("")+drot+"Dateien aus find: "+schwarz+ltoan(verg.size()),1,oblog);
  } else {
    Log(string(rot)+"Arbeite mit opendir in Verzeichnis: "+schwarz+*pfad,obverb,oblog);
    static time_t jetzt = time(0);
    for(int runde=0;runde<2;runde++) {
      DIR *dirHandle = opendir(pfad->c_str());
      struct dirent *dirEntry;
      struct stat entrystat;
      int Dateityp;
      if ( dirHandle != 0) {
        while ( 0 != ( dirEntry = readdir( dirHandle ) ) ) {         //Alle Ordner/Dateien auslesen
          if (lstat((*pfad + vtz + dirEntry->d_name).c_str(), &entrystat)) {
            Log(string("lstat fehlgeschlagen: ") + strerror(errno) + " bei Datei:"+ *pfad+ vtz +string(dirEntry->d_name),1,1,1);
            continue;
          } // (lstat((*pfad + "/" + dirEntry->d_name).c_str(), &entrystat)) 
          Dateityp = entrystat.st_mode & S_IFMT;
          switch (Dateityp) {
            case S_IFDIR:    // Verzeichnis
              if (runde==1) {
                if (strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0) {
                  //		       drot<<*pfad<<"/"<<dirEntry->d_name<<": Verzeichnis\n"; 
                  string aktverz = *pfad+vtz+dirEntry->d_name;
                  beaVerz(&aktverz,wiepfad,minuten,My, lmailadn, lmailsn, lmailempfn, lmailtextn, lmailbodyn, lmailhfeldn, lmailhparn, regexp, geszahl, neuzahl, obopendir, logscreen, keinatt, obverb,oblog);
                } //(strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0) 
              } // switch (Dateityp) 
              break;
            case S_IFREG:   // Datei
              if (runde==0) {
                //		 sql = "insert into \""+dateitb+"\"(parid,dname,groesse,modif) values("+parids+",'"+dirEntry->d_name+"',"+"0"+",'"+ctime(&entrystat.st_mtime)+"')";
                string dateiname;
                /*																												
                                                                          if (strstr(dirEntry->d_name,"'")!=NULL) 
                //		      dateiname = (char*)boost::replace_all_copy(string(dirEntry->d_name),"'","''").c_str();       
                dateiname = ersetze(dirEntry->d_name,"'","''");
                else 
                 */																												
                double minu = difftime(jetzt,entrystat.st_mtime)/60;  
                if (minu<=minuten || !minuten) {
                  dateiname = dirEntry->d_name;
                  string Maildateiname = *pfad+vtz+dateiname;
                  verg.push_back(Maildateiname);
                }
                //																												showMessage(Maildateiname);
              }
              break;
#ifdef linux		   
            case S_IFLNK: if (runde==1) {Log(drot+*pfad+schwarz+": Symb. Link",logscreen,oblog);} break;
            case S_IFSOCK: if (runde==1) {Log(drot+*pfad+schwarz+": Socket",logscreen,oblog);} break;
#endif		  
            case S_IFCHR: if (runde==1) {Log(drot+*pfad+schwarz+": CharDevice",logscreen,oblog);} break;
            case S_IFBLK: if (runde==1) {Log(drot+*pfad+schwarz+": Block-Devc",logscreen,oblog);} break;
#ifndef _MSC_VER
            case S_IFIFO: if (runde==1) {Log(drot+*pfad+schwarz+": named pipe",logscreen,oblog);} break;
#endif
            default:      if (runde==1) {Log(blau+*pfad+schwarz+": Sonstiges",logscreen,oblog);} break;
          } // switch (Dateityp) 
          //																								if (attz) break;
        } // while ( 0 != ( dirEntry = readdir( dirHandle ) ) )         //Alle Ordner/Dateien auslesen
        closedir( dirHandle );         //Den Ordner schliessen
      } // if ( dirHandle != 0)
      //             if (attz) return attz;
    } // for(int runde=0;runde<2;runde++)
  }
  // ZPKT(" nach Inkonsist: ")
  *geszahl+=verg.size();
  for (unsigned nachrnr=0; nachrnr<verg.size(); ++nachrnr){
    //          Log(gelb+verg.at(i)+schwarz,obverb,oblog);
    if (lstat((verg.at(nachrnr).c_str()), &entrystat)) {
      // ZPKT(" nach lstat: ");
      Log(string("lstat fehlgeschlagen: ") + strerror(errno) + " bei Datei:"+ verg.at(nachrnr),1,1,1);
      continue;
    } // (lstat((*pfad + "/" + dirEntry->d_name).c_str(), &entrystat)) 
    //  char*** ferg;
    string vglstr = verg.at(nachrnr);
    ersetzAlle(&vglstr,"'","\\'");
    size_t pos = vglstr.find("))#"),pos0;
    // 1) Wegen des genauen Felduebereinstimmung der ersten gesuchten Felder laueft der erste, laengere select-Befehl schneller
    // 2) alte Mails enthalten nicht unbedingt ein Feld Message-ID oder Content-ID
    RS rschau(My);
    for(unsigned iru=0;iru<3;iru+=1) { // bei iru+=1 statt 2 wuerde noch das Feld Return-Path in lmailhfeld ausprobiert, das geht aber nicht immer
      string messid0,messid1,sql;
      if (pos!=string::npos){
        pos0=vglstr.find("#((");
        messid0=vglstr.substr(pos0+3,pos-pos0-3);
        ersetzAlle(messid0,"´","\"");
        ersetzAlle(messid0,"¯","'");
        ersetzAlle(messid0,"¨",":");
        ersetzAlle(messid0,"(","<");
        ersetzAlle(messid0,")",">");
        ersetzAlle(messid0,"Ì","\\");
        ersetzAlle(messid0,"Í","/");
        ersetzAlle(messid0,"¡","|");
        ersetzAlle(messid0,"¹","*");
        ersetzAlle(messid0,"¿","?");
        ersetzAlle(messid0,"·"," ");
//        messid0 = ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(ersetze(messid0,"´","\""),"¯","'"),"¨",":"),"(","<"),")",">"),"Ì","\\"),"Í","/"),"¡","|"),"¹","*"),"¿","?"),"·"," ");
        vglstr=vglstr.substr(0,pos+3);
        messid1=string("<")+messid0+">";
        switch(iru) {
          case 0: case 1:
            sql=string("select m.id from ")+lmailhfeldn+" f left join lmails m on f.lmailsid = m.id where f.wert =\""+messid1+"\""
              +" and f.name "+(iru?"=\"Return-Path\"":"in (\"Message-ID\",\"Content-ID\")")
              +" and `EML` like \""+vglstr+"%\"";
            break;
          case 2:
            sql=string("SELECT m.id FROM ")+lmailadn+" l left join lmails m ignore index (senderid) on m.senderid = l.id where l.email = \""+messid0+"\""
              +" and `EML` like \""+vglstr+"%\"";
            break;
        }
      } else {
        sql = string("select m.id from `")+lmailsn+"` m where `EML` = \""+vglstr+"\"";
      }
      //          <<"vglstr: "<<rot<<vglstr<<schwarz<<endl;
      // ZPKT(" vor rschau: ");
      //      #undef ZDB
      //      #define ZDB 255
      rschau.Abfrage(sql,ZDB); // ,255);
      //      #undef ZDB
      //      #define ZDB 0
      if (pos==string::npos || (!rschau.obfehl && My->affrows())) break;
    }
    // ZPKT(" nach rschau: ");
    if (!rschau.obfehl) { 
      if (!My->affrows()){ //(ferg=rschau.HolZeile(),!*ferg) 
        (*neuzahl)++;
        //      "rschau.sql: "<<rschau.sql<<endl;
        attz+= zeigNachricht(&verg.at(nachrnr), &entrystat, My, lmailadn, lmailsn, lmailempfn, lmailtextn, lmailbodyn, lmailhfeldn, lmailhparn, regexp, nachrnr, verg.size(), keinatt, obverb,oblog);
        // ZPKT(" nach zeigNachricht: ");
      } else {
        char letzteaend[20];
        tm *loct = localtime(&entrystat.st_mtime);
        strftime(letzteaend,sizeof letzteaend, "%d.%m.%y %X", loct);
        Log(string(obverb?"\n":"")+schwarz+"schon eingetragen "+"("+string(ltoan(nachrnr+1))+"/"+string(ltoan(verg.size()))+") ("+gelb+string(ltoan(entrystat.st_size))+schwarz+" Bytes, "+ schwarz+letzteaend+schwarz+"):\n"+drot+verg.at(nachrnr)+schwarz,1,oblog,1);  
        // ZPKT(" nach schon eingetr: ");
      }
    } // if (!rschau.obfehl) 
    cout<<"My->affrows(): "<<gelb<<My->affrows()<<schwarz<<endl;
    Log("Fertig mit nachrnr: "+violetts+ltoan(nachrnr)+schwarz,obverb,0);
  } // for (unsigned nachrnr=0; nachrnr<verg.size(); ++nachrnr)
  return attz;
} // beaVerz


const char* prueflmails(DB *My,uchar obverb, uchar direkt=0)
{
  if (!direkt){
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("EML","varchar","1","","Dateiname"),
      Feld("EMLu","varchar","1","","urspruenglicher Dateiname"),
      Feld("Datum","datetime","0","0","aus messageParser.getDate()"),
      Feld("Dateidatum","datetime","0","0","Änderungsdatum von EML"),
      Feld("Dateizeit","double","0","0","Änderungsdatum von EML, im time()-Format"),
      Feld("Kopfzeile","varchar","1","",""),
      Feld("SenderID","int","10","",(string("Bezug auf ")+lmailadn).c_str()),
      //    Feld("Sendername","varchar","256","",""),
      //    Feld("Senderemail","varchar","128","",""),
      Feld("Empfaengerzahl","int","10","",""),
      Feld("Anhangzahl","int","10","","")
    };
    Feld ifelder0[] = {Feld("Dateizeit")}; Index i0("dateizeit",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Feld ifelder1[] = {Feld("EML")};       Index i1("EML",ifelder1,sizeof ifelder1/sizeof* ifelder1);
    Feld ifelder2[] = {Feld("EMLu")};      Index i2("EMLu",ifelder2,sizeof ifelder2/sizeof* ifelder2);
    Feld ifelder3[] = {Feld("SenderID")};  Index i3("SenderID",ifelder3,sizeof ifelder3/sizeof* ifelder3);
    Index indices[]={i0,i1,i2,i3};
    // auf jeden Fall ginge "binary" statt "utf8" und "" statt "utf8_general_ci"
    Tabelle tab(lmailsn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,"Mails aus den *.linux1-Dateien im eml-Format, mit vmparse1 ausgelesen","InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab, obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailsn,1,1);
      return NULL;
    }
  }
  return lmailsn;
} // prueflmails

const char* prueflmailad(DB *My,uchar obverb, uchar direkt=0)
{
  if (!direkt){
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("Name","varchar","1","",""),
      Feld("Email","varchar","1","",""),
      Feld("obEmpf","int","1","","0=Sender, 1=Empfaenger",0,0),
      //          Feld("nachr","varchar","263","","Datei",0,0)
    };
    Feld ifelder0[] = {Feld("Name")}; Index i0("Name",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Feld ifelder1[] = {Feld("Email")}; Index i1("Email",ifelder1,sizeof ifelder1/sizeof* ifelder1);
    Index indices[]={i0,i1};
    Tabelle tab(lmailadn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,"Email-Adressen, mit vmp ausgelesen","InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab,obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailadn,1,1);
      return NULL;
    }
  }
  return lmailadn;
}

const char* prueflmailempf(DB *My,uchar obverb,uchar direkt=0)
{
  if (!direkt){
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("EmpfArt","int","10","","Empfaengerart: 0 = normaler, 1 = CC, 2 = BCC",0,0),
      Feld("lmailsID","int","10","",string("Bezug auf ")+lmailsn,0,0),
      Feld("lmailadID","int","10","",(string("Bezug auf ")+lmailadn).c_str(),0,0)
    };
    Feld ifelder0[] = {Feld("lmailsID")}; Index i0("lmailsID",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Feld ifelder1[] = {Feld("lmailadID")}; Index i1("lmailadID",ifelder1,sizeof ifelder1/sizeof* ifelder1);
    Index indices[]={i0,i1};
    Tabelle tab(lmailempfn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,string("Empfaenger, Bezug zwischen ")+lmailsn+" und "+lmailadn,"InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab,obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailempfn,1,1);
      return NULL;
    }
  }
  return lmailempfn;
}

const char* prueflmailhpar(DB *My,uchar obverb, uchar direkt=0)
{
  if (!direkt){
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("lmailsID","int","10","",string("Bezug auf ")+lmailsn,0,0),
      Feld("lfdnr","int","10","","Bezug auf Header-Feld Nr.",0,0),
      Feld("parnr","int","10","","Parameter Nr. zu diesem Header-Feld",0,0),
      Feld("name","varchar","1","","Name des Header-Felds",0,0),
      Feld("wert","varchar","1","","Inhalt des Header-Felds",0,0),
    };
    Feld ifelder0[] = {Feld("lmailsID"),Feld("lfdnr"),Feld("parnr")}; Index i0("lmailsID_lfdnr_parnr",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Feld ifelder1[] = {Feld("name"),Feld("lmailsID")};                Index i1("name_lmailsID",ifelder1,sizeof ifelder1/sizeof* ifelder1);
    Feld ifelder2[] = {Feld("wert")};                                 Index i2("wert",ifelder2,sizeof ifelder2/sizeof* ifelder2);
    Index indices[]={i0,i1,i2};
    // auf jeden Fall ginge "binary" statt "utf8" und "" statt "utf8_general_ci"
    Tabelle tab(lmailhparn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,"Header-Parameter der Mails","InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab, obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailhparn,1,1);
      return NULL;
    }
  }
  return lmailhparn;
} // prueflmailhfeld

const char* prueflmailhfeld(DB *My,uchar obverb, uchar direkt=0)
{
  if (!direkt) {
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("lmailsID","int","10","",string("Bezug auf ")+lmailsn,0,0),
      Feld("lfdnr","int","10","","Header-Feld Nr.",0,0),
      Feld("name","varchar","1","","Name des Header-Felds",0,0),
      Feld("wert","varchar","1","","Inhalt des Header-Felds",0,0),
    };
    Feld ifelder0[] = {Feld("lmailsID"),Feld("lfdnr")}; Index i0("lmailsID_lfdnr",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Feld ifelder1[] = {Feld("name"),Feld("lmailsID")};  Index i1("name_lmailsID",ifelder1,sizeof ifelder1/sizeof* ifelder1);
    Feld ifelder2[] = {Feld("wert","","50")};                   Index i2("wert",ifelder2,sizeof ifelder2/sizeof* ifelder2);
    Index indices[]={i0,i1,i2};
    // auf jeden Fall ginge "binary" statt "utf8" und "" statt "utf8_general_ci"
    Tabelle tab(lmailhfeldn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,"Headerfelder der Mails","InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab, obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailhfeldn,1,1);
      return NULL;
    }
  }
  return lmailhfeldn;
} // prueflmailhfeld

const char* prueflmailbody(DB *My,uchar obverb, uchar direkt=0)
{
  if (!direkt) {
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("lmailsID","int","10","",string("Bezug auf ")+lmailsn,0,0),
      Feld("lfdnr","int","10","","Bodyteil Nr.",0,0),
      Feld("CType","varchar","1","","Body-Field 'Content-Type'",0,0),
      Feld("charset","varchar","1","","parameter 'charset' des Body-Felds 'Content-Type'",0,0),
      Feld("name","varchar","1","","parameter 'name' des Body-Felds 'Content-Type'",0,0),
      Feld("boundary","varchar","1","","parameter 'boundary' des Body-Felds 'Content-Type'",0,0),
      Feld("type","varchar","1","","parameter 'type' des Body-Felds 'Content-Type'",0,0),
      Feld("format","varchar","1","","parameter 'format' des Body-Felds 'Content-Type'",0,0),
      Feld("reply-type","varchar","1","","parameter 'reply-type' des Body-Felds 'Content-Type'",0,0),
      Feld("CTransfer-Encoding","varchar","1","","Body-Field 'Content-Transfer-Encoding'",0,0),
      Feld("CDisposition","varchar","1","","Body-Field 'Content-Disposition'",0,0),
      Feld("filename","varchar","1","","parameter 'filename' des Body-Felds 'Content-Type'",0,0),
    };
    Feld ifelder0[] = {Feld("lmailsID"),Feld("lfdnr")}; Index i0("lmailsID_lfdnr",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Index indices[]={i0};
    // auf jeden Fall ginge "binary" statt "utf8" und "" statt "utf8_general_ci"
    Tabelle tab(lmailbodyn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,"Bodyteile der Mails","InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab, obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailbodyn,1,1);
      return NULL;
    }
  }
  return lmailbodyn;
} // prueflmailbody

const char* prueflmailtext(DB *My,uchar obverb, uchar direkt=0)
{
  if (!direkt) {
    Feld felder[] = {
      Feld("ID","int","10","","eindeutige Identifikation",1,1),
      Feld("lmailsID","int","10","",string("Bezug auf ")+lmailsn,0,0),
      Feld("lfdnr","int","10","","Textteil Nr.",0,0),
      Feld("art","int","2","","0=HTML, 1=Sonstiges",0,0),
      Feld("text","varchar","1","","Inhalt des Textteils"), // mediumtext (text braucht 1% weniger Speicherplatz, fasst 2^16 ggue. 2^24
    };
    Feld ifelder0[] = {Feld("lmailsID"),Feld("lfdnr")}; Index i0("lmailsID_lfdnr",ifelder0,sizeof ifelder0/sizeof* ifelder0);
    Index indices[]={i0};
    // auf jeden Fall ginge "binary" statt "utf8" und "" statt "utf8_general_ci"
    Tabelle tab(lmailtextn,felder,sizeof felder/sizeof* felder,indices,sizeof indices/sizeof *indices,"Textteile der Mails","InnoDB","utf8","utf8_general_ci","DYNAMIC");
    if (My->prueftab(&tab, obverb)) {
      Log(string("Fehler beim Prüfen von: ")+lmailtextn,1,1);
      return NULL;
    }
  }
  return lmailtextn;
} // prueflmailtext
/*
   void try_get_date(const std::string& s)
   _gKLA_
   std::locale::global(std::locale("de_DE.utf8"));
   << "Parsing the date out of '" << s <<
   "' in the locale " << std::locale().name() << '\n';
   std::istringstream str(s);
   std::ios_base::iostate err = std::ios_base::goodbit;

   std::tm t;
   std::istreambuf_iterator<char> ret =
   std::use_facet<std::time_get<char> >(str.getloc()).get_date(
   std::istreambuf_iterator<char>(str),
   std::istreambuf_iterator<char>(),
   str, err, &t
   );
   str.setstate(err);
   if(str) _gKLA_
   << "Day: "   << t.tm_mday << ' '
   << "Month: " << t.tm_mon + 1 << ' '
   << "Year: "  << t.tm_year + 1900 << '\n';
   _gKLZ_ else _gKLA_
   << "Parse failed. Unparsed string: ";
   std::copy(ret, std::istreambuf_iterator<char>(),
   std::ostreambuf_iterator<char>(std::cout));
   << '\n';
   _gKLA_
   _gKLA_
 */
void datumparse(char* rohzp,string *abzp)
{
  struct tm tm;
  memset(&tm, 0, sizeof(struct tm));
  *abzp="";
  // time_t t;
  const char* muster[]={"%Y%m%d%H%M%S","%Y-%m-%d %H:%M:%S","%d.%m.%Y %H:%M:%S","%Y%m%d%H%M","%Y-%m-%d %H:%M","%d.%m.%Y %H:%M","%Y%m%d","%Y-%m-%d","%d.%m.%Y"};
  for(unsigned runde=0;runde<sizeof muster/sizeof *muster;runde++){
    memset(&tm, 0, sizeof(struct tm));
    char* erg=strptime(rohzp, muster[runde], &tm);
    if (erg) {
      //          <<"Distanz: "<<(erg-rohzp)<<endl;
      if (tm.tm_year){
        if (tm.tm_year<0) tm.tm_year+=2000;
        char buffer [20];
        strftime (buffer,sizeof buffer,"%Y%m%d%H%M%S",&tm);
        //          <<"runde: "<<runde<<" buffer: "<<buffer<<endl;
        *abzp=buffer;
        break;
      }
    }
  }
}

int getpar(int argc, char** argv, uchar *obverb, uchar *oblog, uchar *logfilenew, uchar *hilfe, string *mailpfadp, uchar *dodel, uchar *incon, uchar *keinatt, const char** attmusterp, uchar *obopendir, uchar *zeigstand, uchar *dfl, uchar *rueckumb, string *abzp, unsigned long *minuten)
{
  for(int i=argc-1;i>0;i--){if (argv[i][0]==0) argc--;} // damit für das Compilermakro auch im bash-script argc stimmt
  for(int i=1;i<argc;i++) {
    if (0) {
    } else if (!strcmp(argv[i],"-l") || !strcmp(argv[i],"/l") || !strcmp(argv[i],"--log")){
      *oblog=1;
    } else if (!strcmp(argv[i],"-lfn") || !strcmp(argv[i],"/lfn") || !strcmp(argv[i],"--logfilenew") || !strcmp(argv[i],"--logfileneu")){
      *logfilenew=1;
    } else if (!strcmp(argv[i],"-lf") || !strcmp(argv[i],"/lf") || !strcmp(argv[i],"--logfile")){
      if (i<argc-1 && argv[i+1][0]) {
        logdatname=argv[++i];
        logpfad = string(logverz)+logdatname;
        logdatei= logpfad.c_str();
      } else {
        Log(string(drot)+"Fehler: Parameter -lf oder --logfile ohne Datei angegeben!"+schwarz,1,1);
        *hilfe=1;
      }
    } else if (!strcmp(argv[i],"-ap") || !strcmp(argv[i],"/ap") || !strcmp(argv[i],"--attmuster") || !strcmp(argv[i],"--attpattern")){
      if (i<argc-1 && argv[i+1][0]) {
        *attmusterp=argv[++i];
      } else {
        Log(string(drot)+"Fehler: Parameter -ap oder --attmuster ohne Muster angegeben!"+schwarz,1,1);
        *hilfe=1;
      }
    } else if (!strcmp(argv[i],"-ad") || !strcmp(argv[i],"/ad") || !strcmp(argv[i],"--attverz") || !strcmp(argv[i],"--attdir")){
      if (i<argc-1 && argv[i+1][0]) {
        attverz=argv[++i];
        mkdir(attverz.c_str(),S_IRWXU | S_IRGRP | S_IXGRP);
      } else {
        Log(string(drot)+"Fehler: Parameter -ad oder --attdir ohne Verzeichnis angegeben!"+schwarz,1,1);
        *hilfe=1;
      }
    } else if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"/h")|| !strcmp(argv[i],"-?")  || !strcmp(argv[i],"/?") || !strcmp(argv[i],"--hilfe")|| !strcmp(argv[i],"--help")){
      *hilfe=1;
    } else if (!strcmp(argv[i],"-d") || !strcmp(argv[i],"/d") || !strcmp(argv[i],"--delete")){
      *dodel=1;
    } else if (!strcmp(argv[i],"-i") || !strcmp(argv[i],"/i") || !strcmp(argv[i],"--incon")){
      *incon=1;
    } else if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"/v") || !strcmp(argv[i],"--obverb")){
      *obverb=1;
    } else if (!strcmp(argv[i],"-zs") || !strcmp(argv[i],"/zs") || !strcmp(argv[i],"--zeigstandan")){
      *zeigstand=1;
    } else if (!strcmp(argv[i],"-k") || !strcmp(argv[i],"/k") || !strcmp(argv[i],"--keinatt")) {
      *keinatt=1;
    } else if (!strcmp(argv[i],"-od") || !strcmp(argv[i],"/od") || !strcmp(argv[i],"--opendir")) {
      *obopendir=1;
    } else if (!strcmp(argv[i],"-dfl") || !strcmp(argv[i],"/dfl")) {
      *dfl=1;
    } else if (!strcmp(argv[i],"-ru") || !strcmp(argv[i],"/ru") || !strcmp(argv[i],"--rueckumb")){
      *rueckumb=1;
      if (i<argc-1 && argv[i+1][0]) {
        datumparse((char*)argv[++i],abzp);
      }
      if (*abzp==""){
        Log(string(drot)+"Fehler: Parameter -ru oder --rueckumb ohne Zeitpunkt angegeben (Folgeparameter: "+schwarz+argv[i]+drot+")!"+schwarz,1,1);
        *hilfe=1;
      }
    } else if (!strcmp(argv[i],"-m") || !strcmp(argv[i],"/m") || !strcmp(argv[i],"--minuten") || !strcmp(argv[i],"--minutes")) {
      if (i<argc-1 && argv[i+1][0]) {
        *minuten=atol(argv[++i]);
      } else {
        Log(string(drot)+"Fehler: Parameter -m oder --minuten ohne Zahl angegeben!"+schwarz,1,1);
        *hilfe=1;
      }
    } else if (!strcmp(argv[i],"-p") || !strcmp(argv[i],"/p") || !strcmp(argv[i],"--pfad")) {
      if (i<argc-1 && argv[i+1][0]) {
        *mailpfadp=argv[++i];
      } else {
        Log(string(drot)+"Fehler: Parameter -p oder --pfad ohne Zahl angegeben!"+schwarz,1,1);
        *hilfe=1;
      }
      if (mailpfadp->empty()) {
        Log(string(drot)+"Fehler: Parameter -p oder --pfad mit falscher Zahl angegeben!"+schwarz,1,1);
        *hilfe=1;
      }
    } else if (!strcmp(argv[i],"-db") || !strcmp(argv[i],"/db")) {
      if (i<argc-1 && argv[i+1][0]) {
        dbq=argv[++i];
      } else {
        Log(string(drot)+"Fehler: Parameter <dbname> zu -db"+schwarz,1,1);
        *hilfe=1;
      }
      if (!*dbq) {
        Log(string(drot)+"Fehler: Parameter <dbname> zu -db!"+schwarz,1,1);
        *hilfe=1;
      }
    }
  }
  Log(string("Fertig mit Parsen der Befehlszeile"),*obverb,1);
  if (*hilfe){
    cout<<"Gebrauch: "<<drot<<argv[0]<<" [-p <pfad>] [-m <minuten>] [-l] [-v] [-h]"<<schwarz<<endl; 
    cout<<"Parst Mails in Verzeichns (<pfad>), die hoechstens <minuten> alt sind und traegt sie in MySQL-Datenbank '"<<drot<<dbq<<schwarz<<"' ein\n";
    cout<<drot<<" -p <pfad>"<<_schwarz<<": parst die Mails aus aus <pfad> anstatt '"<<drot<<*mailpfadp<<schwarz<<"'\n";
    cout<<drot<<" -m <minuten>"<<_schwarz<<": Beruecksichtigt Mails mit hoechstens <minuten> statt "<<drot<<"10"<<_schwarz<<" (0=alle)\n";
    cout<<drot<<" -d, --delete"<<_schwarz<<": Eintraege zu diesem Verzeichnis und Zeitraum vorher loeschen\n";
    cout<<drot<<" -i, --incon"<<_schwarz<<": Inkonsistenzen durch loeschen des letzten Datensatzes beseitigen\n";
    cout<<drot<<" -v, --obverb"<<_schwarz<<": Bildschirmausgabe gespraechig\n";
    cout<<drot<<" -l, --log"<<_schwarz<<": protokolliert ausfuehrlich in Datei '"<<drot<<logpfad<<_schwarz<<"' (sonst knapper)\n";
    cout<<drot<<" -lf, --logfile <logdatei>"<<_schwarz<<": logdatei wird auf <logdatei> (im Pfad '"<<drot<<logverz<<schwarz<<"') geaendert\n";
    cout<<drot<<" -lfn, --logfilenew"<<_schwarz<<": logdatei vorher loeschen\n";
    cout<<drot<<" -k, --keinatt"<<_schwarz<<" speichert nicht Anhaenge '"<<drot<<*attmusterp<<schwarz<<"' in '"<<drot<<attverz<<schwarz<<"'\n";
    cout<<drot<<" -zs, --zeigstandan"<<_schwarz<<" zeigt vorher den Fuellungszustand der Dateien an "<<"'\n";
    cout<<drot<<" -ap, --attmuster <muster>"<<_schwarz<<": attmuster wird von 'fax.pdf' auf <"<<"muster"<<"> geaendert\n";
    cout<<drot<<" -ad, --attdir <attverz>"<<_schwarz<<": attverz wird von '"<<drot<<attverz<<schwarz<<"' auf <"<<drot<<"attverz"<<"> geaendert\n";
    cout<<drot<<" -od, --opendir"<<_schwarz<<": sucht Dateien mit opendir() anstatt mit find \n";
    cout<<drot<<" -ru <datum>"<<_schwarz<<": benennt die Mails ab <"<<drot<<"datum"<<schwarz<<"> zurueck und loescht sie dann aus der Datenbank\n";
    cout<<drot<<" -db <dbname>"<<_schwarz<<": verwendet die Datenbank <"<<drot<<"dbname"<<schwarz<<"> anstatt \""<<drot<<dbq<<schwarz<<"\"\n";
    cout<<drot<<" -dfl"<<_schwarz<<": Datensaetze fehlender Dateien loeschen\n";
    cout<<drot<<" -h, --hilfe"<<_schwarz<<": Zeigt diesen Bildschirm an\n";
    return 0;
  }
  return 1;
}

void oertliches(const char* host,string *mailpfadp)
{
#ifdef _WIN32
  // char lpszUsername[255]; DWORD dUsername = sizeof(lpszUsername); GetUserName(lpszUsername, &dUsername);
  char cpt[255];DWORD dcpt = sizeof(cpt);
  GetComputerName(cpt,&dcpt);
  // pdfzt="u:\\programmierung\\vs08\\projects\\pdfzutext\\release\\pdfzutext.exe p:\\plz /s";
#elif linux
  // uid_t userId = getuid(); passwd* pw = getpwuid(userId); pw->pw_name<<endl;
  char cpt[MAXHOSTNAMELEN]; size_t cptlen = MAXHOSTNAMELEN;
  gethostname(cpt, cptlen);
  // pdfzt = "/root/bin/pdfzutext /DATA/Patientendokumente/plz /s";
#endif
  if (strstr(cpt,"linux")){
    host = "localhost";
    logverz = "/var/log/";
    if (!strcmp(cpt,"linux2")) {
      out1="/vista/V/out1";
      attverz="/vista/P";
      attanf="/vista/att/att";
      //        pfad="/vista/Mail/EML/gmx7";
      *mailpfadp="/vista/Mail/EML/gmx7"; // "/mnt/tosh/DATA/Mail/EML/gmx7";
    } else {
      out1="/DATA/down/out1";
      attverz="/DATA/Patientendokumente/";
      attanf="/DATA/att/att";
      *mailpfadp="/DATA/Mail/EML/gmx7";
    }
  } else {
    // if (!strcmp(cpt,"ANMELDL") || !strcmp(cpt,"SZSNEU"))
    //			host = "linux1";
    //			host = "[2001:a60:230a:7101:a0a7:25a1:a92:2a5e]";
    host = "192.168.178.21";
    logverz = "C:\\Dokumente und Einstellungen\\All Users\\Anwendungsdaten\\";
  }
  logdatname = "logvmparse.txt";
  logpfad = string(logverz)+logdatname;
  logdatei = logpfad.c_str();
} // oertliches

void dorueckum(DB *My, string *abzp,string *mailpfadp,uchar obverb,uchar oblog)
{
  char ***erg;
  RS rs(My,string("select EMLu, EML from ")+lmailsn+" where EML like \""+*mailpfadp+"%\" and datum >= "+*abzp,255);
  if (!rs.obfehl) while (erg=rs.HolZeile(),*erg) {
    if (*(*erg+1)) if (*(*erg+0)){
      int fehler = rename(*(*erg+1),*(*erg+0));
      Log(string("")+(fehler?drot:schwarz)+(fehler?"erfolglos":"erfolgreich")+schwarz+" von:  "+drot+*(*erg+1)+schwarz,1,oblog);
      Log(string("nach: ")+drot+*(*erg+0)+schwarz,1,oblog);
      if (fehler)
        Log(strerror(errno),1,oblog);
      Log("",1,oblog);
    }
  }
  loescheab(My,mailpfadp,abzp,obverb,oblog);
}

void loeschefehlende(DB *My,uchar obverb,uchar oblog)
{
  char ***erg;
  long gibts[2]={0,0},gibtsnicht=0;
  long zahl=0;
  RS rs(My,string("select EML, EMLu from `")+lmailsn+"`",255);
  if (!rs.obfehl) while(erg=rs.HolZeile(),*erg) {
    // /DATA/Mail/EML/gmx7_/new/E#2011-03-11_17-15-31_+0100#((D08D6900B9996440A802F7090B30DBEB7D4C09@dahsrex01.rka.local))# dauert fast ewig
    if (!rs.obfehl) while(erg=rs.HolZeile(),*erg){
      if (!(++zahl % 100)) {
        Log(string(blau)+"\n"+ltoan(zahl)+schwarz,1,1);
      }
      for(uchar runde=0;runde<2;runde++){
        if (*(*erg+runde)) {
          uchar aktgef=0;
          struct stat eml;
          if (!lstat(*(*erg+runde),&eml)) {
            gibts[runde]++;
            aktgef=1;
          } else {
            string teil(*(*erg+runde));
            size_t pos = teil.rfind("))#");
            if (pos!=string::npos) { 
              teil = teil.substr(0,pos+3);
              string pfad = teil.substr(0,teil.rfind("/")+1);
              ersetzAlle(teil,"$","\\$");
              string cmd= string("find ")+pfad+" -maxdepth 1 -path \""+teil+"*\"";
              Log(string("")+drot+"Suchbefehl: "+schwarz+cmd,obverb,oblog);
              int gefunden=0;
              vector<string> verg;
              if (FILE* pipe = popen(cmd.c_str(), "r")){
                while(!feof(pipe)) {
                  char buffer[1280];
                  if(fgets(buffer, sizeof buffer, pipe) != NULL){
                    size_t posi;
                    if (buffer[posi=strlen(buffer)-1]==10) buffer[posi]=0;
                    verg.push_back(string(buffer));
                  } else {
                  }
                }
                pclose(pipe);
              }
              gefunden=verg.size();
              Log(string("\n")+"cmd: "+cmd,1,1);
              Log(string("gefunden: ")+drot+ltoan((int)gefunden)+schwarz,1,1);
              if (gefunden) {
                cout<<".";
                gibts[runde]++;
                aktgef=1;
              } else if (runde) {
                gibtsnicht++;
              }
            }
          }
          if (runde) {
            if (aktgef)
              Log(string(gelb)+*(*erg+1)+schwarz+" existiert.",1,1);
            else
              Log(string(drot)+*(*erg+1)+schwarz+" existiert auch nicht.",1,1);
          } else {
            if (aktgef)
              cout<<".";
            else
              Log(string(blau)+*(*erg+0)+schwarz+" existiert nicht,",1,1);
          }
          if (aktgef) break;
        }
      }
    }
  }
  Log("",1,1);
  Log(string(drot)+ltoan(gibts[0])+schwarz+" in EML  benannte Dateien gibt es.",1,1);
  Log(string(drot)+ltoan(gibts[1])+schwarz+" in EMLu benannte Dateien gibt es.",1,1);
  Log(string(drot)+ltoan(gibtsnicht)+schwarz+" in EML und EMLu benannte Dateien gibt es nicht.",1,1);
}


int main(int argc, char** argv) 
{
  uchar obverb=0;
  uchar oblog=0;
  uchar logfilenew=0;
  uchar hilfe=0;
  uchar dodel=0;
  uchar incon=0;
  uchar keinatt=0;
  uchar zeigstand=0;
  uchar dfl=0;
  uchar rueckumb=0;
  uchar obopendir=0;
  string abzp; // ab Zeitpunkt
  const char *attmuster = "fax.pdf";
  string mailpfad;
  int attz;
  regex_t regex;
  unsigned long minuten=10;
  const char* host="";
  oertliches(host,&mailpfad);
  if (!getpar(argc, argv, &obverb, &oblog, &logfilenew, &hilfe, &mailpfad, &dodel, &incon, &keinatt, &attmuster, &obopendir, &zeigstand, &dfl, &rueckumb, &abzp, &minuten))
    exit(121);
  // regex kompilieren
  if (!keinatt) {
    int reti=regcomp(&regex, attmuster,REG_EXTENDED); 
    if (reti) {
      Log(string("Der regulaere Ausdruck '")+drot+attmuster+schwarz+"' konnte nicht kompiliert werden. Breche ab!",1,1);
      exit(120);
    } else {
      Log(string("Fertig mit Kompilieren des Musters '")+drot+attmuster+schwarz+"'",obverb || !keinatt,1);
    }
  }
  // VMime initialization
  vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();
  Log(string("Fertig mit Initialisieren von vmime"),obverb,1);
  //    zeigNachricht("/DATA/Mail/EML/gmx7/new/1362009570.M221559P4885Q11011R260209dec3f2b691.linux");
  //																zeigNachricht("/DATA/Mail/EML/Local Folders/Inbox/Brand Chronometer zum ermaessigten Preis.eml",obverb,oblog);
  //																zeigNachricht("/DATA/Mail/EML/gmx2/tmp/1371056174.M246272P17609Q9R5ae09c1553ebb07a.linux1",obverb,oblog);
  double tstart = std::clock(), tende;
  if (logfilenew) remove(logdatei);
  Log(string("Mailpfad: ")+drot+mailpfad+schwarz,1,oblog);
  Log(string("Logpfad: ")+drot+logpfad+schwarz+" (oblog: "+drot+ltoan((int)oblog)+schwarz+")",1,oblog);
  //  DB My;// (MySQL,host,"praxis","sonne",dbq,0,0,0);
  DB My(myDBS,host,"praxis","sonne",dbq,0,0,0,0);
  const char *lmailadn = prueflmailad(&My,obverb);
  const char *lmailsn = prueflmails(&My,obverb);
  const char *lmailempfn = prueflmailempf(&My,obverb);
  const char *lmailtextn = prueflmailtext(&My,obverb);
  const char *lmailbodyn = prueflmailbody(&My,obverb);
  const char *lmailhfeldn = prueflmailhfeld(&My,obverb);
  const char *lmailhparn = prueflmailhpar(&My,obverb);
  if (rueckumb) {
    dorueckum(&My,&abzp,&mailpfad,obverb,oblog);
    exit(119);
  }
  if (dfl) {
    loeschefehlende(&My,obverb,oblog);
    exit(118);
  }
  unsigned long geszahl=0, neuzahl=0;
  uchar wiepfad;
  testepfad(&mailpfad, &wiepfad, &minuten, oblog);
  if (wiepfad) {
    loescheundzeigean(dodel, &My,&mailpfad,minuten,incon, zeigstand, oblog);
    attz=beaVerz(&mailpfad,wiepfad,minuten,&My,lmailadn, lmailsn, lmailempfn, lmailtextn, lmailbodyn, lmailhfeldn, lmailhparn, &regex, &geszahl,&neuzahl, obopendir, 0, keinatt, obverb,oblog); // logscreen,oblog)
  }
  tende = std::clock();
  cout<<rot<<"Pfad: "<<blau<<mailpfad<<rot<<";\nDateien: "<<blau<<geszahl<<rot<<", davon neu eingetragen: "<<blau<<neuzahl<<rot<<"; bearbeitete Anhaenge: "<<blau<<attz<<drot<<";\nZeit: "<<drot<<setprecision(7)<<fixed<<((tende-tstart)/CLOCKS_PER_SEC)<<rot<<setprecision(0)<<" s"<<endl;
  //  Log(string("Pfad: ")+gelb+mailpfad+schwarz+" bearbeitet: "+gelb+ltoan(attz)+drot+" Anhaenge;\nZeit: "+drot+setprecision(7)+fixed+((tende-tstart)/CLOCKS_PER_SEC)+schwarz+setprecision(0)+" s",1,oblog);
  //	   "Fertig!!!!!"<<endl;
#if hilftnix
  cin.clear();
  fflush(stdin);
  fflush(stdout);
  cin.ignore(cin.rdbuf()->in_avail());
#endif
  Log(string("")+"Fertig mit "+blau+argv[0]+schwarz+" !",obverb,oblog);
}  // main
