#ifndef HELPDEFINES_H
#define HELPDEFINES_H

// Constants
#define DATE_FORMAT "yyyy-MM-dd"

// <------------------------ Enable / disable defines ------------------------>
#define TEST_INTERFACE_WITHOUT_API_INTEGRATION // enable/disable
// <------------------------ Enable / disable defines ------------------------>

// <------------------------  Help printing defines   ------------------------>
#define PRINT_DEBUG_JSON(jsonObject) \
    qDebug() << "\n<==============================>"; \
    qDebug() << "BEGIN JSON"; \
    qDebug() << jsonObject; \
    qDebug() << "END JSON"; \
    qDebug() << "<==============================>\n";
// <------------------------  Help printing defines   ------------------------>



// BELOW - READ ONLY SECTION



// <================ CONTROLLED BY TEST_INTERFACE_ONLY ================>
#if defined TEST_INTERFACE_WITHOUT_API_INTEGRATION
    #define TEST_JSON_INPUT // TEST INTERFACE WITHOUT API
#else
    #define USE_API // TEST INTERFACE WITH API
#endif
// <================ CONTROLLED BY TEST_INTERFACE_ONLY ================>


#endif // HELPDEFINES_H
