/*
    Copyright (c) 2006-2018, Alexis Royer, http://alexis.royer.free.fr/CLI

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
          and/or other materials provided with the distribution.
        * Neither the name of the CLI library project nor the names of its contributors may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "cli/pch.h"

#include <stdlib.h>

#include "cli/keyword.h"
#include "cli/endl.h"
#include "cli/command_line.h"
#include "cli/shell.h"
#include "cli/lib_info.h"
#include "config_menu.h"

CLI_NS_USE(cli)


ConfigMenu::ConfigMenu(void)
  : Menu("cli-config", Help()
        .AddHelp(Help::LANG_EN, "CLI settings menu")
        .AddHelp(Help::LANG_FR, "Menu de configuration du CLI")),
    m_pcliShow(NULL), m_pcliShowVersion(NULL), m_pcliShowAuthor(NULL), m_pcliShowLicense(NULL),
    m_pcliEcho(NULL), m_pcliEchoOn(NULL), m_pcliEchoOff(NULL),
    m_pcliBeep(NULL), m_pcliBeepOn(NULL), m_pcliBeepOff(NULL),
    m_pcliLang(NULL), m_pcliEnglishLang(NULL), m_pcliFrenchLang(NULL)
    #ifdef _DEBUG
    , m_pcliCheck(NULL), m_pcliCheckIO(NULL), m_pcliOutChar(NULL)
    #endif
{
}

ConfigMenu::~ConfigMenu(void)
{
}

void ConfigMenu::SetCli(Cli& CLI_Cli)
{
    Menu::SetCli(CLI_Cli);
    {   Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Show CLI information")
            .AddHelp(Help::LANG_FR, "Afficher les informations du CLI"));
        m_pcliShow = dynamic_cast<Keyword*>(& AddElement(new Keyword("show", cli_Help)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Show CLI library version")
                .AddHelp(Help::LANG_FR, "Afficher la version de la librairie CLI"));
            m_pcliShowVersion = dynamic_cast<Keyword*>(& m_pcliShow->AddElement(new Keyword("version", cli_Help)));
            m_pcliShowVersion->AddElement(new Endl(cli_Help)); }
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Show CLI library author")
                .AddHelp(Help::LANG_FR, "Afficher l'auteur de la librairie CLI"));
            m_pcliShowAuthor = dynamic_cast<Keyword*>(& m_pcliShow->AddElement(new Keyword("author", cli_Help)));
            m_pcliShowAuthor->AddElement(new Endl(cli_Help)); }
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Show CLI library license")
                .AddHelp(Help::LANG_FR, "Afficher la licence de la librairie CLI"));
            m_pcliShowLicense = dynamic_cast<Keyword*>(& m_pcliShow->AddElement(new Keyword("license", cli_Help)));
            m_pcliShowLicense->AddElement(new Endl(cli_Help)); }}
    {   Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Modify echo behavior")
            .AddHelp(Help::LANG_FR, "Configuration de l'écho"));
        m_pcliEcho = dynamic_cast<Keyword*>(& AddElement(new Keyword("echo", cli_Help)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Set echo on")
                .AddHelp(Help::LANG_FR, "Activation de l'écho"));
            m_pcliEchoOn = dynamic_cast<Keyword*>(& m_pcliEcho->AddElement(new Keyword("on", cli_Help)));
            m_pcliEchoOn->AddElement(new Endl(cli_Help)); }
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Set echo off")
                .AddHelp(Help::LANG_FR, "Désactivation de l'écho"));
            m_pcliEchoOff = dynamic_cast<Keyword*>(& m_pcliEcho->AddElement(new Keyword("off", cli_Help)));
            m_pcliEchoOff->AddElement(new Endl(cli_Help)); }}
    {   Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Modify beep behavior")
            .AddHelp(Help::LANG_FR, "Configuration du bip"));
        m_pcliBeep = dynamic_cast<Keyword*>(& AddElement(new Keyword("beep", cli_Help)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Set beep on")
                .AddHelp(Help::LANG_FR, "Activation du bip"));
            m_pcliBeepOn = dynamic_cast<Keyword*>(& m_pcliBeep->AddElement(new Keyword("on", cli_Help)));
            m_pcliBeepOn->AddElement(new Endl(cli_Help)); }
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Set beep off")
                .AddHelp(Help::LANG_FR, "Désactivation du bip"));
            m_pcliBeepOff = dynamic_cast<Keyword*>(& m_pcliBeep->AddElement(new Keyword("off", cli_Help)));
            m_pcliBeepOff->AddElement(new Endl(cli_Help)); }}
    {   Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Language setting")
            .AddHelp(Help::LANG_FR, "Modification de la langue"));
        m_pcliLang = dynamic_cast<Keyword*>(& AddElement(new Keyword("lang", cli_Help)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "English language")
                .AddHelp(Help::LANG_FR, "Langue anglaise"));
            m_pcliEnglishLang = dynamic_cast<Keyword*>(& m_pcliLang->AddElement(new Keyword("english", cli_Help)));
            m_pcliEnglishLang->AddElement(new Endl(cli_Help)); }
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "French language")
                .AddHelp(Help::LANG_FR, "Langue française"));
            m_pcliFrenchLang = dynamic_cast<Keyword*>(& m_pcliLang->AddElement(new Keyword("french", cli_Help)));
            m_pcliFrenchLang->AddElement(new Endl(cli_Help)); }}
    #ifdef _DEBUG
    {   Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Check CLI stuff")
            .AddHelp(Help::LANG_FR, "Vérifications du CLI"));
        m_pcliCheck = dynamic_cast<Keyword*>(& AddElement(new Keyword("check", cli_Help)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Check input/output")
                .AddHelp(Help::LANG_FR, "Vérification des entrées/sorties"));
            m_pcliCheckIO = dynamic_cast<Keyword*>(& m_pcliCheck->AddElement(new Keyword("io-device", cli_Help)));
            m_pcliCheckIO->AddElement(new Endl(cli_Help));
            {   Help cli_Help(Help()
                    .AddHelp(Help::LANG_EN, "Check output character")
                    .AddHelp(Help::LANG_FR, "Vérification sorties de caractères"));
                m_pcliOutChar = dynamic_cast<Keyword*>(& m_pcliCheckIO->AddElement(new Keyword("outputs", cli_Help)));
                m_pcliOutChar->AddElement(new Endl(cli_Help)); }}}
    #endif
}

const bool ConfigMenu::ExecuteReserved(const CommandLine& CLI_CmdLine) const
{
    CommandLineIterator it(CLI_CmdLine);

    if (! it.StepIt()) { return false; }
    else if (it == GetShowNode())
    {
        if (! it.StepIt()) { return false; }
        else if (it == GetShowVersionNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                ShowVersion();
                return true;
            }
        }
        else if (it == GetShowAuthorNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                ShowAuthor();
                return true;
            }
        }
        else if (it == GetShowLicenseNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                ShowLicense();
                return true;
            }
        }
    }
    else if (it == GetEchoNode())
    {
        if (! it.StepIt()) { return false; }
        else if (it == GetEchoOnNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                EchoOn();
                return true;
            }
        }
        else if (it == GetEchoOffNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                EchoOff();
                return true;
            }
        }
    }
    else if (it == GetBeepNode())
    {
        if (! it.StepIt()) { return false; }
        else if (it == GetBeepOnNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                BeepOn();
                return true;
            }
        }
        else if (it == GetBeepOffNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                BeepOff();
                return true;
            }
        }
    }
    else if (it == GetLangNode())
    {
        if (! it.StepIt()) { return false; }
        else if (it == GetEnglishLangNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                SetLang(ResourceString::LANG_EN);
                return true;
            }
        }
        else if (it == GetFrenchLangNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                SetLang(ResourceString::LANG_FR);
                return true;
            }
        }
    }
    #ifdef _DEBUG
    else if (it == GetCheckNode())
    {
        if (! it.StepIt()) { return false; }
        else if (it == GetCheckIONode())
        {
            if (! it.StepIt()) { return false; }
            else if (dynamic_cast<const Endl*>(*it))
            {
                CheckIODevice();
                return true;
            }
            else if (it == GetOutCharNode())
            {
                if (! it.StepIt()) { return false; }
                if (dynamic_cast<const Endl*>(*it))
                {
                    CheckOutChar();
                    return true;
                }
            }
        }
    }
    #endif

    return Menu::ExecuteReserved(CLI_CmdLine);
}

const bool ConfigMenu::Execute(const CommandLine& CLI_CmdLine) const
{
    return false;
}

void ConfigMenu::ShowVersion() const
{
    GetOutputStream() << "CLI version " << LIB_VERSION << endl;
}

void ConfigMenu::ShowAuthor() const
{
    GetOutputStream() << LIB_COPYRIGHT << endl;
}

void ConfigMenu::ShowLicense() const
{
    GetOutputStream() << LIB_LICENSE << endl;
}

void ConfigMenu::EchoOn(void) const
{
    GetShell().EnableStream(ECHO_STREAM, true);
    const ResourceString cli_EchoOn = ResourceString()
        .SetString(ResourceString::LANG_EN, "echo is on")
        .SetString(ResourceString::LANG_FR, "écho activé");
    GetOutputStream() << cli_EchoOn.GetString(GetShell().GetLang()) << endl;
}

void ConfigMenu::EchoOff(void) const
{
    GetShell().EnableStream(ECHO_STREAM, false);
    const ResourceString cli_EchoOff = ResourceString()
        .SetString(ResourceString::LANG_EN, "echo is off")
        .SetString(ResourceString::LANG_FR, "écho désactivé");
    GetOutputStream() << cli_EchoOff.GetString(GetShell().GetLang()) << endl;
}

void ConfigMenu::BeepOn(void) const
{
    GetShell().SetBeep(true);
    const ResourceString cli_BeepOn = ResourceString()
        .SetString(ResourceString::LANG_EN, "beep is on")
        .SetString(ResourceString::LANG_FR, "bip activé");
    GetOutputStream() << cli_BeepOn.GetString(GetShell().GetLang()) << endl;
}

void ConfigMenu::BeepOff(void) const
{
    GetShell().SetBeep(false);
    const ResourceString cli_BeepOff = ResourceString()
        .SetString(ResourceString::LANG_EN, "beep is off")
        .SetString(ResourceString::LANG_FR, "bip désactivé");
    GetOutputStream() << cli_BeepOff.GetString(GetShell().GetLang()) << endl;
}

void ConfigMenu::SetLang(const ResourceString::LANG E_Lang) const
{
    GetShell().SetLang(E_Lang);
    const ResourceString cli_SelectedLanguage = ResourceString()
        .SetString(ResourceString::LANG_EN, "English language")
        .SetString(ResourceString::LANG_FR, "Langue française");
    GetOutputStream() << cli_SelectedLanguage.GetString(GetShell().GetLang()) << endl;
}

#ifdef _DEBUG
void ConfigMenu::CheckIODevice(void) const
{
    class Press {
        public: static const bool Key(const Shell& CLI_Shell, const char* const STR_Message, const KEY E_Key) {
            if (E_Key == BREAK)
            {
                CLI_Shell.GetStream(OUTPUT_STREAM) << endl;
                CLI_Shell.GetStream(OUTPUT_STREAM) << "Please follow the instructions, and press one by one the expected characters." << endl;
                CLI_Shell.GetStream(OUTPUT_STREAM) << endl;
            }

            for (KEY e_Key = NULL_KEY; e_Key != E_Key; )
            {
                CLI_Shell.GetStream(OUTPUT_STREAM) << STR_Message << endl;
                e_Key = CLI_Shell.GetInput().GetKey();

                if ((e_Key == NULL_KEY)
                    || ((e_Key == BREAK) && (E_Key != BREAK)))
                {
                    // Abort
                    return false;
                }
                if ((e_Key == ESCAPE) && (E_Key != ESCAPE) && (E_Key != BREAK))
                {
                    // Skip character
                    return true;
                }

                if (e_Key != E_Key)
                {
                    CLI_Shell.GetStream(ERROR_STREAM) << "Invalid key " << (int) e_Key << endl;
                }
            }

            if (E_Key == ESCAPE)
            {
                CLI_Shell.GetStream(OUTPUT_STREAM) << endl;
                CLI_Shell.GetStream(OUTPUT_STREAM) << "From now, press ESCAPE to skip a character entry, press CTRL+C to abort the procedure." << endl;
                CLI_Shell.GetStream(OUTPUT_STREAM) << endl;
            }

            return true;
        }
    };

    // First of all, check break and escape.
    if (! Press::Key(GetShell(), "Press the break sequence (CTRL+C)", BREAK)) return;
    if (! Press::Key(GetShell(), "Press ESCAPE key", ESCAPE)) return;

    // Logout sequence
    if (! Press::Key(GetShell(), "Press the logout sequence (CTRL+D)", LOGOUT)) return;

    // Spacing keys
    if (! Press::Key(GetShell(), "Press the space bar", SPACE)) return;
    if (! Press::Key(GetShell(), "Press TAB key", TAB)) return;
    if (! Press::Key(GetShell(), "Press ENTER key", ENTER)) return;

    // Deletion keys
    if (! Press::Key(GetShell(), "Press BACKSPACE key", BACKSPACE)) return;
    if (! Press::Key(GetShell(), "Press DELETE key (or SUPPR key)", DELETE)) return;
    if (! Press::Key(GetShell(), "Press the clean screen sequence (CTRL+L)", CLS)) return;

    // Navigation keys
    if (! Press::Key(GetShell(), "Press the UP arrow key", KEY_UP)) return;
    if (! Press::Key(GetShell(), "Press the DOWN arrow key", KEY_DOWN)) return;
    if (! Press::Key(GetShell(), "Press the LEFT arrow key", KEY_LEFT)) return;
    if (! Press::Key(GetShell(), "Press the RIGHT arrow key", KEY_RIGHT)) return;
    if (! Press::Key(GetShell(), "Press the PAGE UP key", PAGE_UP)) return;
    if (! Press::Key(GetShell(), "Press the PAGE UP key (CTRL+UP)", PAGE_UP)) return;
    if (! Press::Key(GetShell(), "Press the PAGE DOWN key", PAGE_DOWN)) return;
    if (! Press::Key(GetShell(), "Press the PAGE DOWN key (CTRL+DOWN)", PAGE_DOWN)) return;
    if (! Press::Key(GetShell(), "Press the PAGE LEFT key (if any)", PAGE_LEFT)) return;
    if (! Press::Key(GetShell(), "Press the PAGE LEFT key (CTRL+LEFT)", PAGE_LEFT)) return;
    if (! Press::Key(GetShell(), "Press the PAGE RIGHT key (if any)", PAGE_RIGHT)) return;
    if (! Press::Key(GetShell(), "Press the PAGE RIGHT key (CTRL+RIGHT)", PAGE_RIGHT)) return;
    if (! Press::Key(GetShell(), "Press the BEGIN key (or HOME key)", KEY_BEGIN)) return;
    if (! Press::Key(GetShell(), "Press the BEGIN key (CTRL+A)", KEY_BEGIN)) return;
    if (! Press::Key(GetShell(), "Press the END key", KEY_END)) return;
    if (! Press::Key(GetShell(), "Press the END key (CTRL+E)", KEY_END)) return;
    if (! Press::Key(GetShell(), "Press the PREVIOUS key (ALT+LEFT)", PREVIOUS)) return;
    if (! Press::Key(GetShell(), "Press the PREVIOUS key (CTRL+P)", PREVIOUS)) return;
    if (! Press::Key(GetShell(), "Press the NEXT key (ALT+RIGHT)", NEXT)) return;
    if (! Press::Key(GetShell(), "Press the NEXT key (CTRL+N)", NEXT)) return;

    // Modifiers
    if (! Press::Key(GetShell(), "Press the INSERT key", INSERT)) return;
    if (! Press::Key(GetShell(), "Press the copy sequence (ALT+C)", COPY)) return;
    if (! Press::Key(GetShell(), "Press the copy sequence (CTRL+ALT+C)", COPY)) return;
    if (! Press::Key(GetShell(), "Press the copy sequence (CTRL+INS)", COPY)) return;
    if (! Press::Key(GetShell(), "Press the cut sequence (ALT+X)", CUT)) return;
    if (! Press::Key(GetShell(), "Press the cut sequence (CTRL+X)", CUT)) return;
    if (! Press::Key(GetShell(), "Press the cut sequence (CTRL+ALT+X)", CUT)) return;
    if (! Press::Key(GetShell(), "Press the cut sequence (SHIFT+DELETE)", CUT)) return;
    if (! Press::Key(GetShell(), "Press the cut sequence (CTRL+SHIFT+DELETE)", CUT)) return;
    if (! Press::Key(GetShell(), "Press the paste sequence (ALT+V)", PASTE)) return;
    if (! Press::Key(GetShell(), "Press the paste sequence (CTRL+V)", PASTE)) return;
    if (! Press::Key(GetShell(), "Press the paste sequence (CTRL+ALT+V)", PASTE)) return;
    if (! Press::Key(GetShell(), "Press the paste sequence (SHIFT+INSERT)", PASTE)) return;
    if (! Press::Key(GetShell(), "Press the undo sequence (ALT+Z)", UNDO)) return;
    if (! Press::Key(GetShell(), "Press the undo sequence (CTRL+Z)", UNDO)) return;
    if (! Press::Key(GetShell(), "Press the undo sequence (CTRL+ALT+Z)", UNDO)) return;
    if (! Press::Key(GetShell(), "Press the redo sequence (ALT+Y)", REDO)) return;
    if (! Press::Key(GetShell(), "Press the redo sequence (CTRL+Y)", REDO)) return;
    if (! Press::Key(GetShell(), "Press the redo sequence (CTRL+ALT+Y)", REDO)) return;

    // Number keys
    if (! Press::Key(GetShell(), "Press '0'", KEY_0)) return;
    if (! Press::Key(GetShell(), "Press '1'", KEY_1)) return;
    if (! Press::Key(GetShell(), "Press '2'", KEY_2)) return;
    if (! Press::Key(GetShell(), "Press '3'", KEY_3)) return;
    if (! Press::Key(GetShell(), "Press '4'", KEY_4)) return;
    if (! Press::Key(GetShell(), "Press '5'", KEY_5)) return;
    if (! Press::Key(GetShell(), "Press '6'", KEY_6)) return;
    if (! Press::Key(GetShell(), "Press '7'", KEY_7)) return;
    if (! Press::Key(GetShell(), "Press '8'", KEY_8)) return;
    if (! Press::Key(GetShell(), "Press '9'", KEY_9)) return;
    // Letters
    if (! Press::Key(GetShell(), "Press 'a'", KEY_a)) return;
    if (! Press::Key(GetShell(), "Press 'b'", KEY_b)) return;
    if (! Press::Key(GetShell(), "Press 'c'", KEY_c)) return;
    if (! Press::Key(GetShell(), "Press 'd'", KEY_d)) return;
    if (! Press::Key(GetShell(), "Press 'e'", KEY_e)) return;
    if (! Press::Key(GetShell(), "Press 'f'", KEY_f)) return;
    if (! Press::Key(GetShell(), "Press 'g'", KEY_g)) return;
    if (! Press::Key(GetShell(), "Press 'h'", KEY_h)) return;
    if (! Press::Key(GetShell(), "Press 'i'", KEY_i)) return;
    if (! Press::Key(GetShell(), "Press 'j'", KEY_j)) return;
    if (! Press::Key(GetShell(), "Press 'k'", KEY_k)) return;
    if (! Press::Key(GetShell(), "Press 'l'", KEY_l)) return;
    if (! Press::Key(GetShell(), "Press 'm'", KEY_m)) return;
    if (! Press::Key(GetShell(), "Press 'n'", KEY_n)) return;
    if (! Press::Key(GetShell(), "Press 'o'", KEY_o)) return;
    if (! Press::Key(GetShell(), "Press 'p'", KEY_p)) return;
    if (! Press::Key(GetShell(), "Press 'q'", KEY_q)) return;
    if (! Press::Key(GetShell(), "Press 'r'", KEY_r)) return;
    if (! Press::Key(GetShell(), "Press 's'", KEY_s)) return;
    if (! Press::Key(GetShell(), "Press 't'", KEY_t)) return;
    if (! Press::Key(GetShell(), "Press 'u'", KEY_u)) return;
    if (! Press::Key(GetShell(), "Press 'v'", KEY_v)) return;
    if (! Press::Key(GetShell(), "Press 'w'", KEY_w)) return;
    if (! Press::Key(GetShell(), "Press 'x'", KEY_x)) return;
    if (! Press::Key(GetShell(), "Press 'y'", KEY_y)) return;
    if (! Press::Key(GetShell(), "Press 'z'", KEY_z)) return;
    // Capital letters
    if (! Press::Key(GetShell(), "Press 'A'", KEY_A)) return;
    if (! Press::Key(GetShell(), "Press 'B'", KEY_B)) return;
    if (! Press::Key(GetShell(), "Press 'C'", KEY_C)) return;
    if (! Press::Key(GetShell(), "Press 'D'", KEY_D)) return;
    if (! Press::Key(GetShell(), "Press 'E'", KEY_E)) return;
    if (! Press::Key(GetShell(), "Press 'F'", KEY_F)) return;
    if (! Press::Key(GetShell(), "Press 'G'", KEY_G)) return;
    if (! Press::Key(GetShell(), "Press 'H'", KEY_H)) return;
    if (! Press::Key(GetShell(), "Press 'I'", KEY_I)) return;
    if (! Press::Key(GetShell(), "Press 'J'", KEY_J)) return;
    if (! Press::Key(GetShell(), "Press 'K'", KEY_K)) return;
    if (! Press::Key(GetShell(), "Press 'L'", KEY_L)) return;
    if (! Press::Key(GetShell(), "Press 'M'", KEY_M)) return;
    if (! Press::Key(GetShell(), "Press 'N'", KEY_N)) return;
    if (! Press::Key(GetShell(), "Press 'O'", KEY_O)) return;
    if (! Press::Key(GetShell(), "Press 'P'", KEY_P)) return;
    if (! Press::Key(GetShell(), "Press 'Q'", KEY_Q)) return;
    if (! Press::Key(GetShell(), "Press 'R'", KEY_R)) return;
    if (! Press::Key(GetShell(), "Press 'S'", KEY_S)) return;
    if (! Press::Key(GetShell(), "Press 'T'", KEY_T)) return;
    if (! Press::Key(GetShell(), "Press 'U'", KEY_U)) return;
    if (! Press::Key(GetShell(), "Press 'V'", KEY_V)) return;
    if (! Press::Key(GetShell(), "Press 'W'", KEY_W)) return;
    if (! Press::Key(GetShell(), "Press 'X'", KEY_X)) return;
    if (! Press::Key(GetShell(), "Press 'Y'", KEY_Y)) return;
    if (! Press::Key(GetShell(), "Press 'Z'", KEY_Z)) return;
    // Accented characters
    if (! Press::Key(GetShell(), "Press 'á' ('a' acute)", KEY_aacute)) return;
    if (! Press::Key(GetShell(), "Press 'à' ('a' grave)", KEY_agrave)) return;
    if (! Press::Key(GetShell(), "Press 'ä' ('a' uml)", KEY_auml)) return;
    if (! Press::Key(GetShell(), "Press 'â' ('a' circ)", KEY_acirc)) return;
    if (! Press::Key(GetShell(), "Press 'ç' ('c' cedil)", KEY_ccedil)) return;
    if (! Press::Key(GetShell(), "Press 'é' ('e' acute)", KEY_eacute)) return;
    if (! Press::Key(GetShell(), "Press 'è' ('e' grave)", KEY_egrave)) return;
    if (! Press::Key(GetShell(), "Press 'ë' ('e' uml)", KEY_euml)) return;
    if (! Press::Key(GetShell(), "Press 'ê' ('e' circ)", KEY_ecirc)) return;
    if (! Press::Key(GetShell(), "Press 'í' ('i' acute)", KEY_iacute)) return;
    if (! Press::Key(GetShell(), "Press 'ì' ('i' grave)", KEY_igrave)) return;
    if (! Press::Key(GetShell(), "Press 'ï' ('i' uml)", KEY_iuml)) return;
    if (! Press::Key(GetShell(), "Press 'î' ('i' circ)", KEY_icirc)) return;
    if (! Press::Key(GetShell(), "Press 'ó' ('o' acute)", KEY_oacute)) return;
    if (! Press::Key(GetShell(), "Press 'ò' ('o' grave)", KEY_ograve)) return;
    if (! Press::Key(GetShell(), "Press 'ö' ('o' uml)", KEY_ouml)) return;
    if (! Press::Key(GetShell(), "Press 'ô' ('o' circ)", KEY_ocirc)) return;
    if (! Press::Key(GetShell(), "Press 'ú' ('u' acute)", KEY_uacute)) return;
    if (! Press::Key(GetShell(), "Press 'ù' ('u' grave)", KEY_ugrave)) return;
    if (! Press::Key(GetShell(), "Press 'ü' ('u' uml)", KEY_uuml)) return;
    if (! Press::Key(GetShell(), "Press 'û' ('u' circ)", KEY_ucirc)) return;

    // Special characters / ponctuation
    if (! Press::Key(GetShell(), "Press '+' (plus)", PLUS)) return;
    if (! Press::Key(GetShell(), "Press '-' (minus)", MINUS)) return;
    if (! Press::Key(GetShell(), "Press '*' (star)", STAR)) return;
    if (! Press::Key(GetShell(), "Press '/' (slash)", SLASH)) return;
    if (! Press::Key(GetShell(), "Press '<' (lower than)", LOWER_THAN)) return;
    if (! Press::Key(GetShell(), "Press '>' (greater than)", GREATER_THAN)) return;
    if (! Press::Key(GetShell(), "Press '=' (equal)", EQUAL)) return;
    if (! Press::Key(GetShell(), "Press '%' (percent)", PERCENT)) return;
    if (! Press::Key(GetShell(), "Press '_' (underscore)", UNDERSCORE)) return;
    if (! Press::Key(GetShell(), "Press '@' (arobase)", AROBASE)) return;
    if (! Press::Key(GetShell(), "Press '#' (sharp)", SHARP)) return;
    if (! Press::Key(GetShell(), "Press '&' (ampercent)", AMPERCENT)) return;
    if (! Press::Key(GetShell(), "Press '$' (dollar)", DOLLAR)) return;
    if (! Press::Key(GetShell(), "Press '\\' (backslash)", BACKSLASH)) return;
    if (! Press::Key(GetShell(), "Press '|' (pipe)", PIPE)) return;
    if (! Press::Key(GetShell(), "Press '~' (tilde)", TILDE)) return;
    if (! Press::Key(GetShell(), "Press '²' (square)", SQUARE)) return;
    if (! Press::Key(GetShell(), "Press '€' (euro)", EURO)) return;
    if (! Press::Key(GetShell(), "Press '£' (pound)", POUND)) return;
    if (! Press::Key(GetShell(), "Press 'µ' (micro)", MICRO)) return;
    if (! Press::Key(GetShell(), "Press '§' (paragraph)", PARAGRAPH)) return;
    if (! Press::Key(GetShell(), "Press '°' (degree)", DEGREE)) return;
    if (! Press::Key(GetShell(), "Press '©' (copyright)", COPYRIGHT)) return;
    if (! Press::Key(GetShell(), "Press '?' (question)", QUESTION)) return;
    if (! Press::Key(GetShell(), "Press '!' (exclamation)", EXCLAMATION)) return;
    if (! Press::Key(GetShell(), "Press ':' (column)", COLUMN)) return;
    if (! Press::Key(GetShell(), "Press '.' (dot)", DOT)) return;
    if (! Press::Key(GetShell(), "Press ',' (coma)", COMA)) return;
    if (! Press::Key(GetShell(), "Press ';' (semi-column)", SEMI_COLUMN)) return;
    if (! Press::Key(GetShell(), "Press ' (quote)", QUOTE)) return;
    if (! Press::Key(GetShell(), "Press \" (double quote)", DOUBLE_QUOTE)) return;
    if (! Press::Key(GetShell(), "Press ` (back quote)", BACK_QUOTE)) return;
    if (! Press::Key(GetShell(), "Press '(' (opening brace)", OPENING_BRACE)) return;
    if (! Press::Key(GetShell(), "Press ')' (closing brace)", CLOSING_BRACE)) return;
    if (! Press::Key(GetShell(), "Press '{' (opening curly brace)", OPENING_CURLY_BRACE)) return;
    if (! Press::Key(GetShell(), "Press '}' (closing curly brace)", CLOSING_CURLY_BRACE)) return;
    if (! Press::Key(GetShell(), "Press '[' (opening bracket)", OPENING_BRACKET)) return;
    if (! Press::Key(GetShell(), "Press ']' (closing bracket)", CLOSING_BRACKET)) return;

    // Function keys.
    if (! Press::Key(GetShell(), "Press F1", F1)) return;
    if (! Press::Key(GetShell(), "Press F2", F2)) return;
    if (! Press::Key(GetShell(), "Press F3", F3)) return;
    if (! Press::Key(GetShell(), "Press F4", F4)) return;
    if (! Press::Key(GetShell(), "Press F5", F5)) return;
    if (! Press::Key(GetShell(), "Press F6", F6)) return;
    if (! Press::Key(GetShell(), "Press F7", F7)) return;
    if (! Press::Key(GetShell(), "Press F8", F8)) return;
    if (! Press::Key(GetShell(), "Press F9", F9)) return;
    if (! Press::Key(GetShell(), "Press F10", F10)) return;
    if (! Press::Key(GetShell(), "Press F11", F11)) return;
    if (! Press::Key(GetShell(), "Press F12", F12)) return;
}

void ConfigMenu::CheckOutChar(void) const
{
    GetShell().GetStream(OUTPUT_STREAM) << "Press CTRL+C to abort the following outputs." << endl;

    for (int i=-128; i<128; i++)
    {
        GetShell().GetStream(OUTPUT_STREAM) << i << ": " << (char) i << endl;
        if (GetShell().GetInput().GetKey() == BREAK)
        {
            break;
        }
    }
}
#endif

const Keyword& ConfigMenu::GetShowNode(void) const
{
    return *m_pcliShow;
}

const Keyword& ConfigMenu::GetShowVersionNode(void) const
{
    return *m_pcliShowVersion;
}

const Keyword& ConfigMenu::GetShowAuthorNode(void) const
{
    return *m_pcliShowAuthor;
}

const Keyword& ConfigMenu::GetShowLicenseNode(void) const
{
    return *m_pcliShowLicense;
}

const Keyword& ConfigMenu::GetEchoNode(void) const
{
    return *m_pcliEcho;
}

const Keyword& ConfigMenu::GetEchoOnNode(void) const
{
    return *m_pcliEchoOn;
}

const Keyword& ConfigMenu::GetEchoOffNode(void) const
{
    return *m_pcliEchoOff;
}

const Keyword& ConfigMenu::GetBeepNode(void) const
{
    return *m_pcliBeep;
}

const Keyword& ConfigMenu::GetBeepOnNode(void) const
{
    return *m_pcliBeepOn;
}

const Keyword& ConfigMenu::GetBeepOffNode(void) const
{
    return *m_pcliBeepOff;
}

const Keyword& ConfigMenu::GetLangNode(void) const
{
    return *m_pcliLang;
}

const Keyword& ConfigMenu::GetEnglishLangNode(void) const
{
    return *m_pcliEnglishLang;
}

const Keyword& ConfigMenu::GetFrenchLangNode(void) const
{
    return *m_pcliFrenchLang;
}

#ifdef _DEBUG
const Keyword& ConfigMenu::GetCheckNode(void) const
{
    return *m_pcliCheck;
}

const Keyword& ConfigMenu::GetCheckIONode(void) const
{
    return *m_pcliCheckIO;
}

const Keyword& ConfigMenu::GetOutCharNode(void) const
{
    return *m_pcliOutChar;
}
#endif
