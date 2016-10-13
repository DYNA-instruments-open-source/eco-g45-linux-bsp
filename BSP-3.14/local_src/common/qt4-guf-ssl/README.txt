Notes on SSL, Qt
================

Carsten Behling <carsten.behling@garz-fricke.com>

- It seems that Qt must have a ca-bundle.crt available in its SSL certification directory ("/usr/lib/ssl/certs" in this case).
  It is now simply installed with OpenSSL taken from "Mozilla,org". I used the " mkcabundle.pl" script from the internet to create this file.
- Qt does not care about <hashcode>.0 soft links to the root certificates.
  Instead Qt looks into the directories used in standard distributions like Ubuntu, Redhat, ...)
  for *.crt and *.pem files ("src/network/ssl/qsslsocket-openssl.cpp").
- In this Linux system the SSL diréctory is /usr/lib/ssl and the certificates directory is "/usr/lib/ssl/certs".
- To access "signin.ebay.com" in thes demo over https the following root certificates are needed:
  * VeriSignClass3PublicPrimaryCertificationAuthority-G5.crt
  * VeriSignClass3ExtendedValidationSSLCA.crt
  They are simply installed to "/usr/lib/ssl/certs".
