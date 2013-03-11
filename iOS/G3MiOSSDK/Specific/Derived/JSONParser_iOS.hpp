//
//  JSONParser_iOS.hpp
//  G3MiOSSDK
//
//  Created by Oliver Koehler on 02/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef G3MiOSSDK_JSONParser_iOS_hpp
#define G3MiOSSDK_JSONParser_iOS_hpp

#import <Foundation/Foundation.h>
#import <Foundation/NSJSONSerialization.h>

#include <string>

#include "IJSONParser.hpp"

class JSONParser_iOS : public IJSONParser {
private:
  JSONBaseObject* convert(NSObject *object);

  JSONBaseObject* parse(NSData* jsonData);

public:
  JSONBaseObject* parse(const std::string& string);
  JSONBaseObject* parse(IByteBuffer* buffer);

};



#endif
