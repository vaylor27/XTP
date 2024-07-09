#ifndef XTPTEST_H
#define XTPTEST_H

#include "SimpleLogger.h"
#include "XTP.h"
#include "XTPVulkan.h"

class XTPTest;
class RegisterTestShader;


class XTPTest {
public:
    static SimpleLogger testLogger;

    static std::string executablePath;
    static std::shared_ptr<ShaderObject> testShader;
};
#endif // XTPTEST_H
