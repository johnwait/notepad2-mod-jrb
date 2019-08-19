/*
  Comment lexer modules from "scintilla\src\Catalogue.cxx" not used by Notepad2
  (c) Florian Balmer 2011
*/
  // list of lexer modules to include when compiling Notepad2-mod(-jrb)
  // 2019-08-15: Updated to reflect lexer modules desired for the
  //             Notepad2-mod-jrb fork (JRB)
  var lexers = new Array(
    "lmAHK",
    "lmAsm",
    "lmAU3",
    "lmAVS",
    "lmBash",
    "lmBatch",
    "lmCmake",
    "lmCoffeeScript",
    "lmConf",
    "lmCPP",
    "lmCss",
    "lmDiff",
    "lmHTML",
    "lmInno",
    "lmLatex",
    "lmLua",
    "lmMake",
    "lmMarkdown",
    "lmNsis",
    "lmNull",
    "lmPascal",
    "lmPerl",
    "lmPowerShell",
    "lmProps",
    "lmPython",
    "lmRuby",
    "lmSQL",
    "lmTCL",
    "lmVB",
    "lmVBScript",
    "lmVHDL",
    "lmXML",
    "lmYAML"
  );

  // declares & params
  var targetFile = "scintilla\\src\\Catalogue.cxx",
      keptLexersPH = "<%lexers.join(\"|\")%>",
      keptLexersFilter = "^(\\s*)(LINK_LEXER\\((?!" + keptLexersPH + ")\\w+\\);)",
      projectName = 'Notepad2-mod-jrb',
      scriptTitle = 'Scintilla Lexers Filtering Script - '+projectName,
      CLSID_FSO = 'Scripting.FileSystemObject',
      CLSID_WshShell = 'WScript.Shell';

  /* popup() button combinations */
  var POPUP_STYLE_OKONLY           = 0,
      POPUP_STYLE_OKCANCEL         = 1,
      POPUP_STYLE_ABORTRETRYIGNORE = 2,
      POPUP_STYLE_YESNOCANCEL      = 3,
      POPUP_STYLE_YESNO            = 4,
      POPUP_STYLE_RETRYCANCEL      = 5;
  /* popup() icon types */
  var POPUP_ICON_NONE = 0,
      POPUP_ICON_CRITICAL = 16,
      POPUP_ICON_QUESTION = 32,
      POPUP_ICON_EXCLAMATION = 48,
      POPUP_ICON_INFORMATION = 64;
  /* popup() button IDs */
  var POPUP_BTN_NONE = 0,
      POPUP_BTN_OK = 1,
      POPUP_BTN_CANCEL = 2,
      POPUP_BTN_ABORT = 3,
      POPUP_BTN_RETRY = 4,
      POPUP_BTN_IGNORE = 5,
      POPUP_BTN_YES = 6,
      POPUP_BTN_NO = 7;
  /* popup() method */
  var popup = function(
        msg /*,
        timeoutSecs = 0,
        title = '',
        iconStyle = POPUP_STYLE_OKONLY + POPUP_ICON_NONE
    */) {
        var timeoutSecs = (arguments.length > 1 ? parseInt(arguments[1])+0 : 0),
            title = (arguments.length > 2 ? arguments[2]+'' : ''),
            iconStyle = (arguments.length > 3 ? parseInt(arguments[3])+0 : POPUP_STYLE_OKONLY + POPUP_ICON_NONE);
        return (WScript.CreateObject(CLSID_WshShell)).Popup(
            ''+msg, timeoutSecs, title, iconStyle
          );
    };
  var getScriptPath = function(/* pathToAppend = '' */) {
        return (WScript.CreateObject(CLSID_FSO)).
            GetFile(WScript.ScriptFullName).ParentFolder.Path + (
              arguments.length > 0 ? "\\" + new String(arguments[0]) : ""
            );
    };

  var fso = WScript.CreateObject(CLSID_FSO),
      targetFilePath = getScriptPath(targetFile);

  // make sure user doesn't run this script by accident
  if (POPUP_BTN_YES !== popup(
        "This script will modify the file \""+targetFile+"\" to comment out unwanted "+
          "lexer modules and only keep the desired ones.\n\nDo you wish to proceed?\n\n"+
          "To review or change the list of lexer modules to keep, stop this script and "+
          "open it in a text editor.",
        0,
        scriptTitle,
        POPUP_STYLE_YESNOCANCEL + POPUP_ICON_QUESTION)) {
    WScript.Quit();
  }
  // make sure target file exists
  if (!fso.FileExists(targetFilePath)) {
    popup("The file needing modification, \""+targetFile+"\", could not be found.\n\n"+
          "Make sure the file hasn't been deleted, and check that you're indeed running this "+
          "script at the root of the "+projectName+" project's folder.\n\nScript aborted.",
        0, scriptTitle, POPUP_STYLE_OKONLY + POPUP_ICON_EXCLAMATION);
    WScript.Quit();
  }
  var fh = fso.OpenTextFile(targetFilePath, 1, 0);
  if (!fh.AtEndOfStream) {
    // read content of target file
    var fileContent = fh.ReadAll();
    fh.Close();
    // uncomment all lexers
    fileContent = fileContent.replace(/^(\s*)\/\/(LINK_LEXER)/gim, "$1$2");
    // comment-out undesired ones
    var reExcluded = new RegExp(keptLexersFilter.replace(keptLexersPH, lexers.join("|")), "gim");
    fileContent = fileContent.replace(reExcluded, "$1//$2");
    // replace file content with modified code
    var fh = fso.OpenTextFile(targetFilePath, 2, 0);
    fh.Write(fileContent);
  }
/* end of file "lexlink.js" */