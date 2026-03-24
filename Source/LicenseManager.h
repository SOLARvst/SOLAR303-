#pragma once
#include <JuceHeader.h>

//==============================================================================
// LicenseManager — Gumroad license verification & local storage
// Call verifyWithGumroad() from a background thread only!
//==============================================================================
class LicenseManager
{
public:
    static bool isActivated()
    {
        return getStoredKey().isNotEmpty();
    }

    // Synchronous — must be called from a background thread!
    static bool verifyWithGumroad (const juce::String& key, juce::String& errorMsg)
    {
        auto trimmed = key.trim();

        if (trimmed.isEmpty())
        {
            errorMsg = "Please enter your license key.";
            return false;
        }

        // Developer master key — always valid
        if (trimmed.equalsIgnoreCase ("SOLAR-DEV-2026-ALMA-303"))
        {
            storeKey (trimmed);
            return true;
        }

        juce::URL url ("https://api.gumroad.com/v2/licenses/verify");
        url = url.withParameter ("product_permalink", getProductPermalink());
        url = url.withParameter ("license_key",      trimmed);

        int statusCode = 0;
        juce::StringPairArray responseHeaders;

        auto stream = url.createInputStream (
            juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inPostData)
                .withConnectionTimeoutMs (12000)
                .withStatusCode          (&statusCode)
                .withResponseHeaders     (&responseHeaders)
        );

        if (stream == nullptr)
        {
            errorMsg = "Connection failed. Check your internet connection.";
            return false;
        }

        auto response = stream->readEntireStreamAsString();
        auto json     = juce::JSON::parse (response);

        if (! json.isObject())
        {
            errorMsg = "Invalid response from server.";
            return false;
        }

        if ((bool) json.getProperty ("success", false))
        {
            storeKey (trimmed);
            return true;
        }

        errorMsg = json.getProperty ("message", juce::var ("Invalid license key.")).toString();
        return false;
    }

    static juce::String getStoredKey()
    {
        if (auto f = makePropsFile())
            return f->getValue ("licenseKey");
        return {};
    }

    static void clearLicense()
    {
        if (auto f = makePropsFile())
        {
            f->removeValue ("licenseKey");
            f->saveIfNeeded();
        }
    }

private:
    // ── Replace with your Gumroad product permalink ───────────────────────────
    static juce::String getProductPermalink()
    {
        return "solar303";   // Gumroad product permalink
    }

    static void storeKey (const juce::String& key)
    {
        if (auto f = makePropsFile())
        {
            f->setValue ("licenseKey", key);
            f->saveIfNeeded();
        }
    }

    static std::unique_ptr<juce::PropertiesFile> makePropsFile()
    {
        juce::PropertiesFile::Options opts;
        opts.applicationName     = "SOLAR303";
        opts.filenameSuffix      = "settings";
        opts.folderName          = "SOLAR303";
        opts.osxLibrarySubFolder = "Application Support";
        return std::make_unique<juce::PropertiesFile> (opts);
    }
};
