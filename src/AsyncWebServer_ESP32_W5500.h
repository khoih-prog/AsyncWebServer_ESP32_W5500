/****************************************************************************************************************************
  AsyncWebServer_ESP32_W5500.h - Dead simple Ethernet AsyncWebServer.

  For W5500 LwIP Ethernet in ESP32 (ESP32 + W5500)

  AsyncWebServer_ESP32_W5500 is a library for the LwIP Ethernet W5500 in ESP32 to run AsyncWebServer

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_ESP32_W5500
  Licensed under GPLv3 license

  Original author: Hristo Gochkov

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.

  This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with this library;
  if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version: 1.6.4

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.6.2   K Hoang      30/11/2022 Initial porting for ENC28J60 + ESP32. Sync with AsyncWebServer_WT32_ETH01 v1.6.2
  1.6.3   K Hoang      05/12/2022 Add Async_WebSocketsServer, MQTT examples
  1.6.4   K Hoang      23/12/2022 Remove unused variable to avoid compiler warning and error
 *****************************************************************************************************************************/

#ifndef _AsyncWebServer_ESP32_W5500_H_
#define _AsyncWebServer_ESP32_W5500_H_

/////////////////////////////////////////////////

#if ESP32

  #if (_ASYNC_WEBSERVER_LOGLEVEL_ > 3 )
    #warning Using ESP32 architecture for WebServer_ESP32_W5500
  #endif

#else
  #error This code is designed for ESP32_W5500 to run on ESP32 platform! Please check your Tools->Board setting.
#endif

/////////////////////////////////////////////////

#if ( ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2) ) && ( ARDUINO_ESP32_GIT_VER != 0x46d5afb1 ) )
  #define USING_CORE_ESP32_CORE_V200_PLUS      true

  #if (_ASYNC_WEBSERVER_LOGLEVEL_ > 3 )
    #warning Using code for ESP32 core v2.0.0+ in AsyncWebServer_ESP32_W5500.h
  #endif

  #define ASYNC_WEBSERVER_ESP32_W5500_VERSION      "AsyncWebServer_ESP32_W5500 v1.6.4 for core v2.0.0+"
#else

  #if (_ASYNC_WEBSERVER_LOGLEVEL_ > 3 )
    #warning Using code for ESP32 core v1.0.6- in AsyncWebServer_ESP32_W5500.h
  #endif

  #define ASYNC_WEBSERVER_ESP32_W5500_VERSION      "AsyncWebServer_ESP32_W5500 v1.6.4 for core v1.0.6-"
#endif

#define ASYNC_WEBSERVER_ESP32_W5500_VERSION_MAJOR     1
#define ASYNC_WEBSERVER_ESP32_W5500_VERSION_MINOR     6
#define ASYNC_WEBSERVER_ESP32_W5500_VERSION_PATCH     4

#define ASYNC_WEBSERVER_ESP32_W5500_VERSION_INT       1006004

/////////////////////////////////////////////////

#ifndef ESP32_W5500_AWS_UNUSED
  #define ESP32_W5500_AWS_UNUSED(x)       (void)(x)
#endif

/////////////////////////////////////////////////

#include "Arduino.h"

#include <functional>
#include "FS.h"

#include "StringArray.h"

//////////////////////////////////////////////////////////////
// ESP32_W5500 related code

#include "AsyncWebServer_ESP32_W5500_Debug.h"

#if 1

  #include <WiFi.h>
  #include <WebServer.h> // Introduce corresponding libraries

  #include "w5500/esp32_w5500.h"

  #include <hal/spi_types.h>

  #if !defined(ETH_SPI_HOST)
    #define ETH_SPI_HOST            SPI3_HOST
  #endif

  #if !defined(SPI_CLOCK_MHZ)
    // Using 25MHz for W5500, 14MHz for W5100
    #define SPI_CLOCK_MHZ       25
  #endif

  #if !defined(INT_GPIO)
    #define INT_GPIO            4
  #endif

  #if !defined(MISO_GPIO)
    #define MISO_GPIO           19
  #endif

  #if !defined(MOSI_GPIO)
    #define MOSI_GPIO           23
  #endif

  #if !defined(SCK_GPIO)
    #define SCK_GPIO            18
  #endif

  #if !defined(CS_GPIO)
    #define CS_GPIO             5
  #endif

  #ifndef SHIELD_TYPE
    #define SHIELD_TYPE         "ESP32_W5500"
  #endif



#else

  //#include <WebServer_ESP32_W5500.hpp>


#endif

extern bool ESP32_W5500_eth_connected;

extern void ESP32_W5500_onEvent();

extern void ESP32_W5500_waitForConnect();

extern bool ESP32_W5500_isConnected();

extern void ESP32_W5500_event(WiFiEvent_t event);

//////////////////////////////////////////////////////////////

#include <AsyncTCP.h>

#ifdef ASYNCWEBSERVER_REGEX
  #define ASYNCWEBSERVER_REGEX_ATTRIBUTE
#else
  #define ASYNCWEBSERVER_REGEX_ATTRIBUTE      __attribute__((warning("ASYNCWEBSERVER_REGEX not defined")))
#endif

#define DEBUGF(...) //Serial.printf(__VA_ARGS__)

class AsyncWebServer;
class AsyncWebServerRequest;
class AsyncWebServerResponse;
class AsyncWebHeader;
class AsyncWebParameter;
class AsyncWebRewrite;
class AsyncWebHandler;
class AsyncStaticWebHandler;
class AsyncCallbackWebHandler;
class AsyncResponseStream;

/////////////////////////////////////////////////

#ifndef WEBSERVER_H

typedef enum
{
  HTTP_GET     = 0b00000001,
  HTTP_POST    = 0b00000010,
  HTTP_DELETE  = 0b00000100,
  HTTP_PUT     = 0b00001000,
  HTTP_PATCH   = 0b00010000,
  HTTP_HEAD    = 0b00100000,
  HTTP_OPTIONS = 0b01000000,
  HTTP_ANY     = 0b01111111,
} WebRequestMethod;

#endif

//if this value is returned when asked for data, packet will not be sent and you will be asked for data again
#define RESPONSE_TRY_AGAIN      0xFFFFFFFF

typedef uint8_t WebRequestMethodComposite;
typedef std::function<void(void)> ArDisconnectHandler;

/////////////////////////////////////////////////

/*
   PARAMETER :: Chainable object to hold GET/POST and FILE parameters
 * */

class AsyncWebParameter
{
  private:
    String _name;
    String _value;
    size_t _size;
    bool _isForm;
    bool _isFile;

  public:

    AsyncWebParameter(const String& name, const String& value, bool form = false, bool file = false,
                      size_t size = 0): _name(name), _value(value), _size(size), _isForm(form), _isFile(file)  {}

    /////////////////////////////////////////////////

    inline const String& name() const
    {
      return _name;
    }

    /////////////////////////////////////////////////

    inline const String& value() const
    {
      return _value;
    }

    /////////////////////////////////////////////////

    inline size_t size() const
    {
      return _size;
    }

    /////////////////////////////////////////////////

    inline bool isPost() const
    {
      return _isForm;
    }

    /////////////////////////////////////////////////

    inline bool isFile() const
    {
      return _isFile;
    }

    /////////////////////////////////////////////////

};

/////////////////////////////////////////////////

/*
   HEADER :: Chainable object to hold the headers
 * */

class AsyncWebHeader
{
  private:
    String _name;
    String _value;

  public:
    AsyncWebHeader(const String& name, const String& value): _name(name), _value(value) {}

    /////////////////////////////////////////////////

    AsyncWebHeader(const String& data): _name(), _value()
    {
      if (!data)
        return;

      int index = data.indexOf(':');

      if (index < 0)
        return;

      _name = data.substring(0, index);
      _value = data.substring(index + 2);
    }

    /////////////////////////////////////////////////

    ~AsyncWebHeader() {}

    /////////////////////////////////////////////////

    inline const String& name() const
    {
      return _name;
    }

    /////////////////////////////////////////////////

    inline const String& value() const
    {
      return _value;
    }

    /////////////////////////////////////////////////

    inline String toString() const
    {
      return String(_name + ": " + _value + "\r\n");
    }

    /////////////////////////////////////////////////

};

/////////////////////////////////////////////////

/*
   REQUEST :: Each incoming Client is wrapped inside a Request and both live together until disconnect
 * */

typedef enum
{
  RCT_NOT_USED = -1,
  RCT_DEFAULT = 0,
  RCT_HTTP,
  RCT_WS,
  RCT_EVENT,
  RCT_MAX
} RequestedConnectionType;

typedef std::function<size_t(uint8_t*, size_t, size_t)> AwsResponseFiller;
typedef std::function<String(const String&)> AwsTemplateProcessor;

/////////////////////////////////////////////////

class AsyncWebServerRequest
{
    using File = fs::File;
    using FS = fs::FS;
    friend class AsyncWebServer;
    friend class AsyncCallbackWebHandler;

  private:
    AsyncClient* _client;
    AsyncWebServer* _server;
    AsyncWebHandler* _handler;
    AsyncWebServerResponse* _response;
    StringArray _interestingHeaders;
    ArDisconnectHandler _onDisconnectfn;

    String _temp;
    uint8_t _parseState;

    uint8_t _version;
    WebRequestMethodComposite _method;
    String _url;
    String _host;
    String _contentType;
    String _boundary;
    String _authorization;
    RequestedConnectionType _reqconntype;
    void _removeNotInterestingHeaders();
    bool _isDigest;
    bool _isMultipart;
    bool _isPlainPost;
    bool _expectingContinue;
    size_t _contentLength;
    size_t _parsedLength;

    LinkedList<AsyncWebHeader *> _headers;
    LinkedList<AsyncWebParameter *> _params;
    LinkedList<String *> _pathParams;

    uint8_t _multiParseState;
    uint8_t _boundaryPosition;
    size_t _itemStartIndex;
    size_t _itemSize;
    String _itemName;
    String _itemFilename;
    String _itemType;
    String _itemValue;
    uint8_t *_itemBuffer;
    size_t _itemBufferIndex;
    bool _itemIsFile;

    void _onPoll();
    void _onAck(size_t len, uint32_t time);
    void _onError(int8_t error);
    void _onTimeout(uint32_t time);
    void _onDisconnect();
    void _onData(void *buf, size_t len);

    void _addParam(AsyncWebParameter*);
    void _addPathParam(const char *param);

    bool _parseReqHead();
    bool _parseReqHeader();
    void _parseLine();
    void _parsePlainPostChar(uint8_t data);
    void _parseMultipartPostByte(uint8_t data, bool last);
    void _addGetParams(const String& params);

    void _handleUploadStart();
    void _handleUploadByte(uint8_t data, bool last);
    void _handleUploadEnd();

  public:
    File _tempFile;
    void *_tempObject;

    AsyncWebServerRequest(AsyncWebServer*, AsyncClient*);
    ~AsyncWebServerRequest();

    /////////////////////////////////////////////////

    inline AsyncClient* client()
    {
      return _client;
    }

    /////////////////////////////////////////////////

    inline uint8_t version() const
    {
      return _version;
    }

    /////////////////////////////////////////////////

    inline WebRequestMethodComposite method() const
    {
      return _method;
    }

    /////////////////////////////////////////////////

    inline const String& url() const
    {
      return _url;
    }

    /////////////////////////////////////////////////

    inline const String& host() const
    {
      return _host;
    }

    /////////////////////////////////////////////////

    inline const String& contentType() const
    {
      return _contentType;
    }

    /////////////////////////////////////////////////

    inline size_t contentLength() const
    {
      return _contentLength;
    }

    /////////////////////////////////////////////////

    inline bool multipart() const
    {
      return _isMultipart;
    }

    /////////////////////////////////////////////////

    const char * methodToString() const;
    const char * requestedConnTypeToString() const;

    /////////////////////////////////////////////////

    inline RequestedConnectionType requestedConnType() const
    {
      return _reqconntype;
    }

    /////////////////////////////////////////////////

    bool isExpectedRequestedConnType(RequestedConnectionType erct1, RequestedConnectionType erct2 = RCT_NOT_USED,
                                     RequestedConnectionType erct3 = RCT_NOT_USED);
    void onDisconnect (ArDisconnectHandler fn);

    //hash is the string representation of:
    // base64(user:pass) for basic or
    // user:realm:md5(user:realm:pass) for digest
    bool authenticate(const char * hash);
    bool authenticate(const char * username, const char * password, const char * realm = NULL, bool passwordIsHash = false);
    void requestAuthentication(const char * realm = NULL, bool isDigest = true);

    /////////////////////////////////////////////////

    inline void setHandler(AsyncWebHandler *handler)
    {
      _handler = handler;
    }

    /////////////////////////////////////////////////

    void addInterestingHeader(const String& name);

    void redirect(const String& url);

    void send(AsyncWebServerResponse *response);
    void send(int code, const String& contentType = String(), const String& content = String());

    void send(int code, const String& contentType, const char *content, bool nonDetructiveSend = true);    // RSMOD

    void send(FS &fs, const String& path, const String& contentType = String(), bool download = false,
              AwsTemplateProcessor callback = nullptr);
    void send(File content, const String& path, const String& contentType = String(), bool download = false,
              AwsTemplateProcessor callback = nullptr);
    void send(Stream &stream, const String& contentType, size_t len, AwsTemplateProcessor callback = nullptr);
    void send(const String& contentType, size_t len, AwsResponseFiller callback,
              AwsTemplateProcessor templateCallback = nullptr);

    void sendChunked(const String& contentType, AwsResponseFiller callback,
                     AwsTemplateProcessor templateCallback = nullptr);

    void send_P(int code, const String& contentType, const uint8_t * content, size_t len,
                AwsTemplateProcessor callback = nullptr);
    void send_P(int code, const String& contentType, PGM_P content, AwsTemplateProcessor callback = nullptr);

    AsyncWebServerResponse *beginResponse(int code, const String& contentType = String(), const String& content = String());

    AsyncWebServerResponse *beginResponse(int code, const String& contentType, const char * content = nullptr); // RSMOD

    // KH add
    AsyncWebServerResponse *beginResponse(int code, const String& contentType, const uint8_t * content, size_t len,
                                          AwsTemplateProcessor callback = nullptr);
    //////


    AsyncWebServerResponse *beginResponse(FS &fs, const String& path, const String& contentType = String(),
                                          bool download = false,
                                          AwsTemplateProcessor callback = nullptr);
    AsyncWebServerResponse *beginResponse(File content, const String& path, const String& contentType = String(),
                                          bool download = false,
                                          AwsTemplateProcessor callback = nullptr);
    AsyncWebServerResponse *beginResponse(Stream &stream, const String& contentType, size_t len,
                                          AwsTemplateProcessor callback = nullptr);
    AsyncWebServerResponse *beginResponse(const String& contentType, size_t len, AwsResponseFiller callback,
                                          AwsTemplateProcessor templateCallback = nullptr);

    AsyncWebServerResponse *beginChunkedResponse(const String& contentType, AwsResponseFiller callback,
                                                 AwsTemplateProcessor templateCallback = nullptr);
    AsyncResponseStream *beginResponseStream(const String& contentType, size_t bufferSize = 1460);

    AsyncWebServerResponse *beginResponse_P(int code, const String& contentType, const uint8_t * content, size_t len,
                                            AwsTemplateProcessor callback = nullptr);
    AsyncWebServerResponse *beginResponse_P(int code, const String& contentType, PGM_P content,
                                            AwsTemplateProcessor callback = nullptr);

    size_t headers() const;                     // get header count
    bool hasHeader(const String& name) const;   // check if header exists
    bool hasHeader(const __FlashStringHelper * data) const;   // check if header exists

    AsyncWebHeader* getHeader(const String& name) const;
    AsyncWebHeader* getHeader(const __FlashStringHelper * data) const;
    AsyncWebHeader* getHeader(size_t num) const;

    size_t params() const;                      // get arguments count
    bool hasParam(const String& name, bool post = false, bool file = false) const;
    bool hasParam(const __FlashStringHelper * data, bool post = false, bool file = false) const;

    AsyncWebParameter* getParam(const String& name, bool post = false, bool file = false) const;
    AsyncWebParameter* getParam(const __FlashStringHelper * data, bool post, bool file) const;
    AsyncWebParameter* getParam(size_t num) const;

    /////////////////////////////////////////////////

    inline size_t args() const
    {
      return params();  // get arguments count
    }

    /////////////////////////////////////////////////

    const String& arg(const String& name) const; // get request argument value by name
    const String& arg(const __FlashStringHelper * data) const; // get request argument value by F(name)
    const String& arg(size_t i) const;           // get request argument value by number
    const String& argName(size_t i) const;       // get request argument name by number
    bool hasArg(const char* name) const;         // check if argument exists
    bool hasArg(const __FlashStringHelper * data) const;         // check if F(argument) exists

    const String& ASYNCWEBSERVER_REGEX_ATTRIBUTE pathArg(size_t i) const;

    const String& header(const char* name) const;// get request header value by name
    const String& header(const __FlashStringHelper * data) const;// get request header value by F(name)
    const String& header(size_t i) const;        // get request header value by number
    const String& headerName(size_t i) const;    // get request header name by number
    String urlDecode(const String& text) const;
};

/////////////////////////////////////////////////

/*
   FILTER :: Callback to filter AsyncWebRewrite and AsyncWebHandler (done by the Server)
 * */

typedef std::function<bool(AsyncWebServerRequest *request)> ArRequestFilterFunction;

bool ON_STA_FILTER(AsyncWebServerRequest *request);

bool ON_AP_FILTER(AsyncWebServerRequest *request);

/////////////////////////////////////////////////

/*
   REWRITE :: One instance can be handle any Request (done by the Server)
 * */

class AsyncWebRewrite
{
  protected:
    String _from;
    String _toUrl;
    String _params;
    ArRequestFilterFunction _filter;

  public:

    /////////////////////////////////////////////////

    AsyncWebRewrite(const char* from, const char* to): _from(from), _toUrl(to), _params(String()), _filter(NULL)
    {
      int index = _toUrl.indexOf('?');

      if (index > 0)
      {
        _params = _toUrl.substring(index + 1);
        _toUrl = _toUrl.substring(0, index);
      }
    }

    /////////////////////////////////////////////////

    virtual ~AsyncWebRewrite() {}

    /////////////////////////////////////////////////

    inline AsyncWebRewrite& setFilter(ArRequestFilterFunction fn)
    {
      _filter = fn;
      return *this;
    }

    /////////////////////////////////////////////////

    inline bool filter(AsyncWebServerRequest *request) const
    {
      return _filter == NULL || _filter(request);
    }

    /////////////////////////////////////////////////

    inline const String& from() const
    {
      return _from;
    }

    /////////////////////////////////////////////////

    inline const String& toUrl() const
    {
      return _toUrl;
    }

    /////////////////////////////////////////////////

    inline const String& params() const
    {
      return _params;
    }

    /////////////////////////////////////////////////

    virtual bool match(AsyncWebServerRequest *request)
    {
      return from() == request->url() && filter(request);
    }
};

/////////////////////////////////////////////////

/*
   HANDLER :: One instance can be attached to any Request (done by the Server)
 * */

class AsyncWebHandler
{
  protected:
    ArRequestFilterFunction _filter;
    String _username;
    String _password;

  public:
    AsyncWebHandler(): _username(""), _password("") {}

    /////////////////////////////////////////////////

    inline AsyncWebHandler& setFilter(ArRequestFilterFunction fn)
    {
      _filter = fn;
      return *this;
    }

    /////////////////////////////////////////////////

    inline AsyncWebHandler& setAuthentication(const char *username, const char *password)
    {
      _username = String(username);
      _password = String(password);
      return *this;
    };

    /////////////////////////////////////////////////

    inline bool filter(AsyncWebServerRequest *request)
    {
      return _filter == NULL || _filter(request);
    }

    /////////////////////////////////////////////////

    virtual ~AsyncWebHandler() {}

    /////////////////////////////////////////////////

    virtual bool canHandle(AsyncWebServerRequest *request __attribute__((unused)))
    {
      return false;
    }

    /////////////////////////////////////////////////

    virtual void handleRequest(AsyncWebServerRequest *request __attribute__((unused))) {}
    virtual void handleUpload(AsyncWebServerRequest *request  __attribute__((unused)),
                              const String& filename __attribute__((unused)),
                              size_t index __attribute__((unused)), uint8_t *data __attribute__((unused)), size_t len __attribute__((unused)),
                              bool final  __attribute__((unused))) {}
    virtual void handleBody(AsyncWebServerRequest *request __attribute__((unused)), uint8_t *data __attribute__((unused)),
                            size_t len __attribute__((unused)), size_t index __attribute__((unused)), size_t total __attribute__((unused))) {}

    /////////////////////////////////////////////////

    virtual bool isRequestHandlerTrivial()
    {
      return true;
    }
};

/////////////////////////////////////////////////

/*
   RESPONSE :: One instance is created for each Request (attached by the Handler)
 * */

typedef enum
{
  RESPONSE_SETUP,
  RESPONSE_HEADERS,
  RESPONSE_CONTENT,
  RESPONSE_WAIT_ACK,
  RESPONSE_END,
  RESPONSE_FAILED
} WebResponseState;

/////////////////////////////////////////////////

class AsyncWebServerResponse
{
  protected:
    int _code;
    LinkedList<AsyncWebHeader *> _headers;
    String _contentType;
    size_t _contentLength;
    bool _sendContentLength;
    bool _chunked;
    size_t _headLength;
    size_t _sentLength;
    size_t _ackedLength;
    size_t _writtenLength;
    WebResponseState _state;
    const char* _responseCodeToString(int code);

  public:
    AsyncWebServerResponse();
    virtual ~AsyncWebServerResponse();
    virtual void setCode(int code);
    virtual void setContentLength(size_t len);
    virtual void setContentType(const String& type);
    virtual void addHeader(const String& name, const String& value);
    virtual String _assembleHead(uint8_t version);
    virtual bool _started() const;
    virtual bool _finished() const;
    virtual bool _failed() const;
    virtual bool _sourceValid() const;
    virtual void _respond(AsyncWebServerRequest *request);
    virtual size_t _ack(AsyncWebServerRequest *request, size_t len, uint32_t time);
};

/////////////////////////////////////////////////

/*
   SERVER :: One instance
 * */

typedef std::function<void(AsyncWebServerRequest *request)> ArRequestHandlerFunction;
typedef std::function<void(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)>
ArUploadHandlerFunction;
typedef std::function<void(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)>
ArBodyHandlerFunction;

/////////////////////////////////////////////////

class AsyncWebServer
{
  protected:
    AsyncServer _server;
    LinkedList<AsyncWebRewrite*> _rewrites;
    LinkedList<AsyncWebHandler*> _handlers;
    AsyncCallbackWebHandler* _catchAllHandler;

  public:
    AsyncWebServer(uint16_t port);
    ~AsyncWebServer();

    void begin();
    void end();

#if ASYNC_TCP_SSL_ENABLED
    void onSslFileRequest(AcSSlFileHandler cb, void* arg);
    void beginSecure(const char *cert, const char *private_key_file, const char *password);
#endif

    AsyncWebRewrite& addRewrite(AsyncWebRewrite* rewrite);
    bool removeRewrite(AsyncWebRewrite* rewrite);
    AsyncWebRewrite& rewrite(const char* from, const char* to);

    AsyncWebHandler& addHandler(AsyncWebHandler* handler);
    bool removeHandler(AsyncWebHandler* handler);

    AsyncCallbackWebHandler& on(const char* uri, ArRequestHandlerFunction onRequest);
    AsyncCallbackWebHandler& on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);
    AsyncCallbackWebHandler& on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest,
                                ArUploadHandlerFunction onUpload);
    AsyncCallbackWebHandler& on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest,
                                ArUploadHandlerFunction onUpload, ArBodyHandlerFunction onBody);

    AsyncStaticWebHandler& serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_control = NULL);

    void onNotFound(ArRequestHandlerFunction fn);  //called when handler is not assigned
    void onFileUpload(ArUploadHandlerFunction fn); //handle file uploads
    void onRequestBody(ArBodyHandlerFunction
                       fn); //handle posts with plain body content (JSON often transmitted this way as a request)

    void reset(); //remove all writers and handlers, with onNotFound/onFileUpload/onRequestBody

    void _handleDisconnect(AsyncWebServerRequest *request);
    void _attachHandler(AsyncWebServerRequest *request);
    void _rewriteRequest(AsyncWebServerRequest *request);
};

/////////////////////////////////////////////////

class DefaultHeaders
{
    using headers_t = LinkedList<AsyncWebHeader *>;
    headers_t _headers;

    /////////////////////////////////////////////////

    DefaultHeaders()
      : _headers(headers_t([](AsyncWebHeader * h)
    {
      delete h;
    }))
    {}

    /////////////////////////////////////////////////

  public:
    using ConstIterator = headers_t::ConstIterator;

    /////////////////////////////////////////////////

    void addHeader(const String& name, const String& value)
    {
      _headers.add(new AsyncWebHeader(name, value));
    }

    /////////////////////////////////////////////////

    inline ConstIterator begin() const
    {
      return _headers.begin();
    }

    /////////////////////////////////////////////////

    inline ConstIterator end() const
    {
      return _headers.end();
    }

    /////////////////////////////////////////////////

    DefaultHeaders(DefaultHeaders const &) = delete;
    DefaultHeaders &operator=(DefaultHeaders const &) = delete;

    /////////////////////////////////////////////////

    static DefaultHeaders &Instance()
    {
      static DefaultHeaders instance;
      return instance;
    }
};

/////////////////////////////////////////////////

#include "WebResponseImpl.h"
#include "WebHandlerImpl.h"
#include "AsyncWebSocket.h"
#include "AsyncEventSource.h"

#endif /* _AsyncWebServer_ESP32_W5500_H_ */
