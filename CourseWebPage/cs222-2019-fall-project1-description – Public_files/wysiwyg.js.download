(function($, window, document) {

var TracWysiwyg = function(textarea, options) {
    var self = this;
    var editorMode = TracWysiwyg.getEditorMode();
    textarea.setAttribute('data-tracwysiwyg-initialized', 'doing');
    this.autolink = true;
    this.textarea = textarea;
    this.options = options = options || {};
    var wikitextToolbar = null;
    var textareaResizable = null;
    if (/\btrac-resizable\b/i.test(textarea.className)) {
        var tmp = textarea.parentNode;
        tmp = tmp && tmp.parentNode;
        if (tmp && /\btrac-resizable\b/i.test(tmp.className)) {
            wikitextToolbar = tmp.previousSibling;
            textareaResizable = tmp;
        }
    }
    else {
        wikitextToolbar = textarea.previousSibling;
    }
    if (wikitextToolbar && (wikitextToolbar.nodeType != 1 || wikitextToolbar.className != "wikitoolbar")) {
        wikitextToolbar = null;
    }
    this.textareaResizable = textareaResizable;
    this.wikitextToolbar = wikitextToolbar;

    this.createEditable(document, textarea, textareaResizable);
    var frame = this.frame;
    var resizable = this.resizable;

    this.contentWindow = frame.contentWindow;
    this.contentDocument = this.contentWindow.document;

    this.initializeEditor(this.contentDocument);
    this.wysiwygToolbar = this.createWysiwygToolbar(document);
    this.styleMenu = this.createStyleMenu(document);
    this.decorationMenu = this.createDecorationMenu(document);
    this.tableMenu = this.createTableMenu(document);
    this.menus = [ this.styleMenu, this.decorationMenu, this.tableMenu ];
    this.toolbarButtons = this.setupMenuEvents();
    this.toggleEditorButtons = null;
    this.autolinkButton = null;
    this.savedWysiwygHTML = null;

    this.setupToggleEditorButtons();
    this.setupSyncTextAreaHeight();

    var styleStatic = { position: "static", left: "-9999px", top: "-9999px" };
    var styleAbsolute = { position: "absolute", left: "-9999px", top: "-9999px" };
    switch (editorMode) {
    case "textarea":
        TracWysiwyg.setStyle(textareaResizable || textarea, styleStatic);
        if (wikitextToolbar) {
            TracWysiwyg.setStyle(wikitextToolbar, styleStatic);
        }
        TracWysiwyg.setStyle(resizable || frame, { position: "absolute",
            left: "-9999px", top: TracWysiwyg.elementPosition(textareaResizable || textarea).top + "px" });
        TracWysiwyg.setStyle(this.wysiwygToolbar, styleAbsolute);
        TracWysiwyg.setStyle(this.autolinkButton.parentNode, { display: "none" });
        textarea.setAttribute("tabIndex", "");
        frame.setAttribute("tabIndex", "-1");
        break;
    case "wysiwyg":
        TracWysiwyg.setStyle(textareaResizable || textarea, { position: "absolute",
            left: "-9999px", top: TracWysiwyg.elementPosition(textareaResizable || textarea).top + "px" });
        if (wikitextToolbar) {
            TracWysiwyg.setStyle(wikitextToolbar, styleAbsolute);
        }
        TracWysiwyg.setStyle(resizable || frame, styleStatic);
        TracWysiwyg.setStyle(this.wysiwygToolbar, styleStatic);
        TracWysiwyg.setStyle(this.autolinkButton.parentNode, { display: "" });
        textarea.setAttribute("tabIndex", "-1");
        frame.setAttribute("tabIndex", "");
        break;
    }

    var body = document.body;
    for (var i = 0; i < this.menus.length; i++) {
        body.insertBefore(this.menus[i], body.firstChild);
    }
    var element = wikitextToolbar || textareaResizable || textarea;
    element.parentNode.insertBefore(this.toggleEditorButtons, element);
    element.parentNode.insertBefore(this.wysiwygToolbar, element);

    function lazySetup() {
        if (self.contentDocument.body) {
            var exception;
            try { self.execCommand("useCSS", false); } catch (e) { }
            try { self.execCommand("styleWithCSS", false); } catch (e) { }
            if (editorMode == "wysiwyg") {
                try { self.loadWysiwygDocument() } catch (e) { exception = e }
            }
            self.setupEditorEvents();
            self.setupFormEvent();
            if (exception) {
                (self.textareaResizable || self.textarea).style.position = "static";
                if (self.wikitextToolbar) {
                    self.wikitextToolbar.style.position = "static";
                }
                (self.resizable || self.frame).style.position = self.wysiwygToolbar.style.position = "absolute";
                self.autolinkButton.parentNode.style.display = "none";
                alert("Failed to activate the wysiwyg editor.");
                throw exception;
            }
            else {
                textarea.setAttribute('data-tracwysiwyg-initialized', 'done');
            }
        }
        else {
            setTimeout(lazySetup, 100);
        }
    }
    lazySetup();
};

var prototype = TracWysiwyg.prototype;

prototype.initializeEditor = function(d) {
    var l = window.location;
    var html = [];
    html.push(
        '<!DOCTYPE html PUBLIC',
        ' "-//W3C//DTD XHTML 1.0 Transitional//EN"',
        ' "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">\n',
        '<html xmlns="http://www.w3.org/1999/xhtml">',
        '<head>',
        '<base href="', l.protocol, '//', l.host, '/" />',
        '<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />');
    var stylesheets = TracWysiwyg.tracPaths.stylesheets;
    if (!stylesheets) {
        // Work around wysiwyg stops with Agilo
        var base = TracWysiwyg.tracPaths.base.replace(/\/*$/, "/");
        stylesheets = [ base + "chrome/common/css/trac.css", base + "chrome/tracwysiwyg/editor.css" ];
    }
    var length = stylesheets.length;
    for (var i = 0; i < length; i++) {
        html.push('<link rel="stylesheet" href="' + stylesheets[i] + '" type="text/css" />');
    }
    html.push('<title></title>', '</head>', '<body></body>', '</html>');

    var first = !window.opera && d.addEventListener ? true : false;
    if (first) {
        d.designMode = "On";
    }
    d.open();
    d.write(html.join(""));
    d.close();
    if (!first) {
        d.designMode = "On";
        if (d != this.contentWindow.document) {
            this.contentDocument = this.contentWindow.document;
        }
    }
};

prototype.toggleAutolink = function() {
    this.autolink = !this.autolink;
    this.autolinkButton.checked = this.autolink;
};

prototype.listenerToggleAutolink = function(input) {
    var self = this;
    return function(event) {
        self.autolink = input.checked;
    };
};

prototype.listenerToggleEditor = function(selected, unselected) {
    function toggle() {
        unselected.removeAttribute('checked');
        unselected.checked = false;
        selected.setAttribute('checked', 'checked');
        selected.checked = true;
    }
    var self = this;
    var type = selected.value;

    switch (type) {
    case "textarea":
        return function(event) {
            toggle();
            var textarea = self.textareaResizable || self.textarea;
            if (textarea.style.position == "absolute") {
                self.hideAllMenus();
                self.loadTracWikiText();
                textarea.style.position = "static";
                self.textarea.setAttribute("tabIndex", "");
                if (self.wikitextToolbar) {
                    self.wikitextToolbar.style.position = "static";
                }
                self.syncTextAreaHeight();
                (self.resizable || self.frame).style.position = self.wysiwygToolbar.style.position = "absolute";
                self.frame.setAttribute("tabIndex", "-1");
                self.autolinkButton.parentNode.style.display = "none";
                TracWysiwyg.setEditorMode(type);
            }
            self.focusTextarea();
        };
    case "wysiwyg":
        return function(event) {
            toggle();
            var frame = self.resizable || self.frame;
            if (frame.style.position == "absolute") {
                try {
                    self.loadWysiwygDocument();
                }
                catch (e) {
                    TracWysiwyg.stopEvent(event || window.event);
                    alert("Failed to activate the wysiwyg editor.");
                    throw e;
                }
                (self.textareaResizable || self.textarea).style.position = "absolute";
                self.textarea.setAttribute("tabIndex", "-1");
                if (self.wikitextToolbar) {
                    self.wikitextToolbar.style.position = "absolute";
                }
                frame.style.position = self.wysiwygToolbar.style.position = "static";
                self.frame.setAttribute("tabIndex", "");
                self.autolinkButton.parentNode.style.display = "";
                TracWysiwyg.setEditorMode(type);
            }
            self.focusWysiwyg();
        };
    }
};

prototype.activeEditor = function() {
    return this.textarea.style.position == "absolute" ? "wysiwyg" : "textarea";
};

prototype.setupFormEvent = function() {
    var self = this;

    function listener(event) {
        var textarea = self.textareaResizable || self.textarea;
        try {
            if (textarea.style.position == "absolute") {
                var body = self.contentDocument.body;
                if (self.savedWysiwygHTML !== null && body.innerHTML != self.savedWysiwygHTML) {
                    self.textarea.value = self.domToWikitext(body, self.options);
                }
            }
        }
        catch (e) {
            TracWysiwyg.stopEvent(event || window.event);
        }
    }
    $(this.textarea.form).bind("submit", listener);
};

prototype.createEditable = function(d, textarea, textareaResizable) {
    var self = this;
    var getStyle = TracWysiwyg.getStyle;
    var dimension = getDimension(textarea);
    if (!dimension.height) {
        setTimeout(lazy, 100);
    }
    if (!dimension.height) {
        dimension.height = parseInt(getStyle(textarea, "lineHeight"), 10) * (textarea.rows || 3);
    }
    var wrapper = d.createElement("div");
    wrapper.innerHTML = jQuery.htmlFormat(''
        + '<iframe class="wysiwyg" src="javascript:\'\'" width="100%" '
        + 'height="$1" frameborder="0" marginwidth="0" marginheight="0">'
        + '</iframe>', dimension.height);
    var frame = this.frame = wrapper.firstChild;

    if (textareaResizable) {
        var offset = null;
        var offsetFrame = null;
        var contentDocument = null;
        var grip = d.createElement("div");
        grip.className = "trac-grip";
        $(grip).bind("mousedown", beginDrag);
        wrapper.appendChild(grip);
        var resizable = d.createElement("div");
        resizable.className = "trac-resizable";
        resizable.appendChild(wrapper);
        grip.style.marginLeft = (frame.offsetLeft - grip.offsetLeft) + 'px';
        grip.style.marginRight = (grip.offsetWidth - frame.offsetWidth) +'px';
        this.resizable = resizable;
        textareaResizable.parentNode.insertBefore(resizable, textareaResizable.nextSibling);
    }
    else {
        textarea.parentNode.insertBefore(frame, textarea.nextSibling);
    }

    function beginDrag(event) {
        offset = frame.height - event.pageY;
        contentDocument = self.contentDocument;
        frame.blur();
        $(d).bind({mousemove: dragging, mouseup: endDrag});
        $(contentDocument).bind({mousemove: draggingForFrame, mouseup: endDrag});
    }

    var topPageY = 0, framePageY = 0;
    function dragging(event) {
        var height = Math.max(32, offset + event.pageY);
        textarea.style.height = height + "px";
        frame.height = height;
    }

    function draggingForFrame(event) {
        var height = Math.max(32, event.clientY);
        textarea.style.height = height + "px";
        frame.height = height;
    }

    function endDrag(event) {
        self.focusWysiwyg();
        $(d).unbind("mousemove", dragging).unbind(d, "mouseup", endDrag);
        $(contentDocument).unbind("mousemove", draggingForFrame)
                          .unbind("mouseup", endDrag);
    }

    function getDimension(textarea) {
        var width = textarea.offsetWidth;
        if (width) {
            var parentWidth = textarea.parentNode.offsetWidth
                            + parseInt(getStyle(textarea, 'borderLeftWidth'), 10)
                            + parseInt(getStyle(textarea, 'borderRightWidth'), 10);
            if (width == parentWidth) {
                width = "100%";
            }
        }
        return { width: width, height: textarea.offsetHeight };
    }

    function lazy() {
        var dimension = getDimension(textarea);
        if (dimension.height) {
            self.frame.height = dimension.height;
        }
        else {
            setTimeout(lazy, 100);
        }
    }
};

prototype.createWysiwygToolbar = function(d) {
    var html = [
        '<ul>',
        '<li class="wysiwyg-menu-style" title="Style">',
        '<a id="wt-style" href="#">',
        '<span class="wysiwyg-menu-style">Style</span>',
        '<span class="wysiwyg-menu-paragraph">Normal</span>',
        '<span class="wysiwyg-menu-heading1">Header 1</span>',
        '<span class="wysiwyg-menu-heading2">Header 2</span>',
        '<span class="wysiwyg-menu-heading3">Header 3</span>',
        '<span class="wysiwyg-menu-heading4">Header 4</span>',
        '<span class="wysiwyg-menu-heading5">Header 5</span>',
        '<span class="wysiwyg-menu-heading6">Header 6</span>',
        '<span class="wysiwyg-menu-code">Code block</span>',
        '<span class="wysiwyg-menu-quote">Quote</span>',
        '</a></li>',
        '<li title="Bold (Ctrl+B)"><a id="wt-strong" href="#"></a></li>',
        '<li title="Italic (Ctrl+I)"><a id="wt-em" href="#"></a></li>',
        '<li title="Underline (Ctrl+U)"><a id="wt-underline" href="#"></a></li>',
        '<li title="Monospace"><a id="wt-monospace" href="#"></a></li>',
        '<li><a id="wt-decorationmenu" href="#"></a></li>',
        '<li title="Remove format"><a id="wt-remove" href="#"></a></li>',
        '<li title="Link"><a id="wt-link" href="#"></a></li>',
        '<li title="Unlink"><a id="wt-unlink" href="#"></a></li>',
        '<li title="Ordered list"><a id="wt-ol" href="#"></a></li>',
        '<li title="List"><a id="wt-ul" href="#"></a></li>',
        '<li title="Outdent"><a id="wt-outdent" href="#"></a></li>',
        '<li title="Indent"><a id="wt-indent" href="#"></a></li>',
        '<li title="Table"><a id="wt-table" href="#"></a></li>',
        '<li><a id="wt-tablemenu" href="#"></a></li>',
        '<li title="Horizontal rule"><a id="wt-hr" href="#"></a></li>',
        '<li title="Line break (Shift+Enter)"><a id="wt-br" href="#"></a></li>',
        '</ul>' ];
    var div = d.createElement("div");
    div.className = "wysiwyg-toolbar";
    div.innerHTML = html.join("").replace(/ href="#">/g, ' href="#" onmousedown="return false" tabindex="-1">');
    return div;
};

prototype.createStyleMenu = function(d) {
    var html = [
        '<p><a id="wt-paragraph" href="#">Normal</a></p>',
        '<h1><a id="wt-heading1" href="#">Header 1</a></h1>',
        '<h2><a id="wt-heading2" href="#">Header 2</a></h2>',
        '<h3><a id="wt-heading3" href="#">Header 3</a></h3>',
        '<h4><a id="wt-heading4" href="#">Header 4</a></h4>',
        '<h5><a id="wt-heading5" href="#">Header 5</a></h5>',
        '<h6><a id="wt-heading6" href="#">Header 6</a></h6>',
        '<pre class="wiki"><a id="wt-code" href="#">Code block</a></pre>',
        '<blockquote class="citation"><a id="wt-quote" href="#">Quote</a></blockquote>' ];
    var menu = d.createElement("div");
    menu.className = "wysiwyg-menu";
    TracWysiwyg.setStyle(menu, { position: "absolute", left: "-1000px", top: "-1000px", zIndex: 1000 });
    menu.innerHTML = html.join("").replace(/ href="#">/g, ' href="#" onmousedown="return false" tabindex="-1">');
    return menu;
};

prototype.createDecorationMenu = function(d) {
    var html = [
        '<ul>',
        '<li><a id="wt-strike" href="#">Strike through</a></li>',
        '<li><a id="wt-sup" href="#">Superscript</a></li>',
        '<li><a id="wt-sub" href="#">Subscript</a></li>',
        '</ul>' ];
    var menu = d.createElement("div");
    menu.className = "wysiwyg-menu";
    TracWysiwyg.setStyle(menu, { position: "absolute", left: "-1000px", top: "-1000px", zIndex: 1000 });
    menu.innerHTML = html.join("").replace(/ href="#">/g, ' href="#" onmousedown="return false" tabindex="-1">');
    return menu;
};

prototype.createTableMenu = function(d) {
    var html = [
        '<ul>',
        '<li><a id="wt-insert-row-before" href="#">Insert row before</a></li>',
        '<li><a id="wt-insert-row-after" href="#">Insert row after</a></li>',
        '<li><a id="wt-insert-col-before" href="#">Insert column before</a></li>',
        '<li><a id="wt-insert-col-after" href="#">Insert column after</a></li>',
        '<li><a id="wt-delete-row" href="#">Delete row</a></li>',
        '<li><a id="wt-delete-col" href="#">Delete column</a></li>',
        '</ul>' ];
    var menu = d.createElement("div");
    menu.className = "wysiwyg-menu";
    TracWysiwyg.setStyle(menu, { position: "absolute", left: "-1000px", top: "-1000px", zIndex: 1000 });
    menu.innerHTML = html.join("").replace(/ href="#">/g, ' href="#" onmousedown="return false" tabindex="-1">');
    return menu;
};

prototype.setupMenuEvents = function() {
    function addToolbarEvent(element, self, args) {
        var method = args.shift();
        $(element).bind("click", function(event) {
            var w = self.contentWindow;
            TracWysiwyg.stopEvent(event || w.event);
            var keepMenus = false;
            try {
                keepMenus = method.apply(self, args);
            }
            finally {
                if (!keepMenus) {
                    self.hideAllMenus();
                }
                element.blur();
                w.focus();
            }
        });
    }

    function argsByType(self, name, element) {
        switch (name) {
        case "style":       return [ self.toggleMenu, self.styleMenu, element ];
        case "strong":      return [ self.execDecorate, "bold" ];
        case "em":          return [ self.execDecorate, "italic" ];
        case "underline":   return [ self.execDecorate, "underline" ];
        case "strike":      return [ self.execDecorate, "strikethrough" ];
        case "sub":         return [ self.execDecorate, "subscript" ];
        case "sup":         return [ self.execDecorate, "superscript" ];
        case "monospace":   return [ self.execDecorate, "monospace" ];
        case "decorationmenu":  return [ self.toggleMenu, self.decorationMenu, element ];
        case "remove":      return [ self.execCommand, "removeformat" ];
        case "paragraph":   return [ self.formatParagraph ];
        case "heading1":    return [ self.formatHeaderBlock, "h1" ];
        case "heading2":    return [ self.formatHeaderBlock, "h2" ];
        case "heading3":    return [ self.formatHeaderBlock, "h3" ];
        case "heading4":    return [ self.formatHeaderBlock, "h4" ];
        case "heading5":    return [ self.formatHeaderBlock, "h5" ];
        case "heading6":    return [ self.formatHeaderBlock, "h6" ];
        case "link":        return [ self.createLink ];
        case "unlink":      return [ self.execCommand, "unlink" ];
        case "ol":          return [ self.insertOrderedList ];
        case "ul":          return [ self.insertUnorderedList ];
        case "outdent":     return [ self.outdent ];
        case "indent":      return [ self.indent ];
        case "table":       return [ self.insertTable ];
        case "tablemenu":   return [ self.toggleMenu, self.tableMenu, element ];
        case "insert-row-before":   return [ self.insertTableRow, false ];
        case "insert-row-after":    return [ self.insertTableRow, true ];
        case "insert-col-before":   return [ self.insertTableColumn, false ];
        case "insert-col-after":    return [ self.insertTableColumn, true ];
        case "delete-row":  return [ self.deleteTableRow ];
        case "delete-col":  return [ self.deleteTableColumn ];
        case "code":        return [ self.formatCodeBlock ];
        case "quote":       return [ self.formatQuoteBlock ];
        case "hr":          return [ self.insertHorizontalRule ];
        case "br":          return [ self.insertLineBreak ];
        }
        return null;
    }

    function setup(container) {
        var elements = container.getElementsByTagName("a");
        var length = elements.length;
        for (var i = 0; i < length; i++) {
            var element = elements[i];
            var name = element.id.replace(/^wt-/, "");
            var args = argsByType(this, name, element);
            if (args) {
                addToolbarEvent(element, this, args);
                buttons[name] = element;
            }
        }
    }

    var buttons = {};
    setup.call(this, this.wysiwygToolbar);
    for (var i = 0; i < this.menus.length; i++) {
        setup.call(this, this.menus[i]);
    }
    return buttons;
};

prototype.toggleMenu = function(menu, element) {
    if (parseInt(menu.style.left, 10) < 0) {
        this.hideAllMenus(menu);
        var position = TracWysiwyg.elementPosition(element);
        TracWysiwyg.setStyle(menu, { left: position[0] + "px", top: (position[1] + 18) + "px" });
    }
    else {
        this.hideAllMenus();
    }
    return true;
};

prototype.hideAllMenus = function(except) {
    var menus = this.menus;
    var length = menus.length;
    for (var i = 0; i < length; i++) {
        if (menus[i] != except) {
            TracWysiwyg.setStyle(menus[i], { left: "-1000px", top: "-1000px" });
        }
    }
};

prototype.execDecorate = function(name) {
    if (this.selectionContainsTagName("pre")) {
        return;
    }
    var getSelfOrAncestor = TracWysiwyg.getSelfOrAncestor;
    var position = this.getSelectionPosition();
    var ancestor = {};
    ancestor.start = getSelfOrAncestor(position.start, /^(?:a|tt)$/);
    ancestor.end = getSelfOrAncestor(position.end, /^(?:a|tt)$/);
    this.expandSelectionToElement(ancestor);

    if (name != "monospace") {
        this.execCommand(name);
    }
    else {
        this.execDecorateMonospace();
    }
    this.selectionChanged();
};

prototype.execDecorateMonospace = function() {
    var html = this.getSelectionHTML();
    var removePattern = /<tt.*?>|<\/tt>/gi;
    if (/^<tt.*?>/i.test(html) && /<\/tt>$/i.test(html)) {
        html = html.replace(removePattern, "");
    }
    else {
        var id = this.generateDomId();
        html = '<tt id="' + id + '">' + html.replace(removePattern, "") + "</tt>";
    }
    this.insertHTML(html);
    var node = this.contentDocument.getElementById(id);
    if (node) {
        this.selectNode(node);
    }
};

prototype.execCommand = function(name, arg) {
    return this.contentDocument.execCommand(name, false, arg);
};

prototype.setupEditorEvents = function() {
    var getSelfOrAncestor = TracWysiwyg.getSelfOrAncestor;
    var self = this;
    var $d = $(this.contentDocument);
    var ime = false;

    function listenerKeydown(event) {
        var method = null;
        var args = null;
        event = event || self.contentWindow.event;
        var keyCode = event.keyCode;
        switch (keyCode) {
        case 0x09:  // TAB
            var range = self.getSelectionRange();
            var stop = false;
            var element = getSelfOrAncestor(range.startContainer, /^(?:li|pre|table)$/);
            if (element) {
                switch (element.tagName.toLowerCase()) {
                case "li":
                    self.execCommand(event.shiftKey ? "outdent" : "indent");
                    self.selectionChanged();
                    stop = true;
                    break;
                case "pre":
                    self.insertHTML("\t");
                    stop = true;
                    break;
                case "table":
                    if (getSelfOrAncestor(range.endContainer, "table") == element) {
                        self.moveFocusInTable(!event.shiftKey);
                        self.selectionChanged();
                        stop = true;
                    }
                    break;
                }
            }
            if (stop) {
                TracWysiwyg.stopEvent(event);
            }
            return;
        case 0xe5:
            ime = true;
            break;
        }
        switch ((keyCode & 0x00fffff) | (event.ctrlKey ? 0x40000000 : 0)
            | (event.shiftKey ? 0x20000000 : 0) | (event.altKey ? 0x10000000 : 0))
        {
        case 0x40000042:  // C-b
            method = self.execDecorate;
            args = [ "bold" ];
            break;
        case 0x40000049:  // C-i
            method = self.execDecorate;
            args = [ "italic" ];
            break;
        case 0x4000004c:  // C-l
            method = self.toggleAutolink;
            args = [];
            break;
        case 0x40000055:  // C-u
            method = self.execDecorate;
            args = [ "underline" ];
            break;
        case 0x40000059:  // C-y
            method = self.execCommand;
            args = [ "redo" ];
            break;
        case 0x4000005a:  // C-z
            method = self.execCommand;
            args = [ "undo" ];
            break;
        }
        if (method !== null) {
            TracWysiwyg.stopEvent(event);
            method.apply(self, args);
            self.selectionChanged();
        }
        else if (keyCode) {
            var focus = self.getFocusNode();
            if (!getSelfOrAncestor(focus, /^(?:p|li|h[1-6]|t[dh]|d[td]|pre|blockquote)$/)) {
                self.execCommand("formatblock", "<p>");
            }
        }
    }
    $d.bind(window.opera ? "keypress" : "keydown", listenerKeydown);

    function listenerKeypress(event) {
        event = event || self.contentWindow.event;
        var modifier = (event.ctrlKey ? 0x40000000 : 0)
            | (event.shiftKey ? 0x20000000 : 0) | (event.altKey ? 0x10000000 : 0);
        switch (event.charCode || event.keyCode) {
        case 0x20:  // SPACE
            self.detectTracLink(event);
            return;
        case 0x3e:  // ">"
            self.detectTracLink(event);
            return;
        case 0x0d:  // ENTER
            self.detectTracLink(event);
            switch (modifier) {
            case 0:
                if (self.insertParagraphOnEnter) {
                    self.insertParagraphOnEnter(event);
                }
                break;
            case 0x20000000:    // Shift
                if (self.insertLineBreakOnShiftEnter) {
                    self.insertLineBreakOnShiftEnter(event);
                }
                break;
            }
            return;
        }
    }
    $d.bind("keypress", listenerKeypress);

    function listenerKeyup(event) {
        var keyCode = event.keyCode;
        if (ime) {
            switch (keyCode) {
            case 0x20:  // SPACE
                self.detectTracLink(event);
                break;
            }
            ime = false;
        }
        self.selectionChanged();
    }
    $d.bind("keyup", listenerKeyup);

    function listenerMouseup(event) {
        self.selectionChanged();
    }
    $d.bind("mouseup", listenerMouseup);

    function listenerClick(event) {
        self.hideAllMenus();
        self.selectionChanged();
    }
    $d.bind("click", listenerClick);
};

prototype.loadWysiwygDocument = function() {
    var d = this.contentDocument;
    var container = d.body;
    var tmp;

    while (tmp = container.lastChild) {
        container.removeChild(tmp);
    }
    var fragment = this.wikitextToFragment(this.textarea.value, d, this.options);
    container.appendChild(fragment);
    this.savedWysiwygHTML = container.innerHTML;
};

prototype.focusWysiwyg = function() {
    var self = this;
    var w = this.contentWindow;
    function lazy() {
        w.focus();
        try { self.execCommand("useCSS", false); } catch (e) { }
        try { self.execCommand("styleWithCSS", false); } catch (e) { }
        self.selectionChanged();
    }
    setTimeout(lazy, 10);
};

prototype.loadTracWikiText = function() {
    this.textarea.value = this.domToWikitext(this.contentDocument.body, this.options);
    this.savedWysiwygHTML = null;
};

prototype.focusTextarea = function() {
    this.textarea.focus();
};

prototype.setupToggleEditorButtons = function() {
    var div = document.createElement("div");
    var mode = TracWysiwyg.editorMode;
    var html = ''
        + '<label title="Links as you type (Ctrl-L)">'
        + '<input type="checkbox" checked="checked" />'
        + 'autolink</label> '
        + '<label><input type="radio" value="wysiwyg"'
        + (mode == "wysiwyg" ? ' checked="checked"' : '')
        + ' />wysiwyg</label> '
        + '<label><input type="radio" value="textarea"'
        + (mode == "textarea" ? ' checked="checked"' : '')
        + ' />textarea</label> '
        + '&nbsp; ';
    div.className = "editor-toggle";
    div.innerHTML = html;
    this.toggleEditorButtons = div;

    var buttons = div.getElementsByTagName("input");

    var autolink = buttons[0];
    var listener = this.listenerToggleAutolink(autolink);
    $(autolink).bind({click: listener, keypress: listener});
    this.autolinkButton = autolink;

    var wysiwyg = buttons[1];
    var textarea = buttons[2];
    $(wysiwyg).bind("click", this.listenerToggleEditor(wysiwyg, textarea));
    $(textarea).bind("click", this.listenerToggleEditor(textarea, wysiwyg));
};

prototype.setupSyncTextAreaHeight = function() {
    var self = this;
    var timer = null;

    var editrows = document.getElementById("editrows");
    if (editrows) {
        $(editrows).bind("change", changeHeight);
    }
    if (this.textareaResizable) {
        $(this.textarea.nextSibling).bind("mousedown", beginDrag);
    }

    function changeHeight() {
        if (timer !== null) {
            clearTimeout(timer);
        }
        setTimeout(sync, 10);
    }

    function beginDrag(event) {
        $(document).bind({mousemove: changeHeight, mouseup: endDrag});
    }

    function endDrag(event) {
        $(document).unbind("mousemove", changeHeight)
                   .unbind("mouseup", endDrag);
    }

    function sync() {
        timer = null;
        self.syncTextAreaHeight();
    }
};

prototype.syncTextAreaHeight = function() {
    var height = this.textarea.offsetHeight;
    var frame = this.frame;
    if (height > 0 && frame.height != height) {
        frame.height = height;
    }
};

prototype.detectTracLink = function(event) {
    if (!this.autolink) {
        return;
    }
    var range = this.getSelectionRange();
    var node = range.startContainer;
    if (!node || !range.collapsed) {
        return;
    }
    var getSelfOrAncestor = TracWysiwyg.getSelfOrAncestor;
    if (getSelfOrAncestor(node, /^(?:a|tt|pre)$/)) {
        return;
    }

    var offset = range.startOffset;
    if (node.nodeType != 3) {
        node = node.childNodes[offset];
        while (node && node.nodeType != 3) {
            node = node.lastChild;
        }
        if (!node) {
            return;
        }
        offset = node.nodeValue.length;
    }
    else if (offset == 0) {
        node = node.previousSibling;
        if (!node || node.nodeType == 1) {
            return;
        }
        offset = node.nodeValue.length;
    }
    var startContainer = node;
    var endContainer = node;
    var text = [ node.nodeValue.substring(0, offset) ];
    for ( ; ; ) {
        if (/[ \t\r\n\f\v]/.test(text[text.length - 1])) {
            break;
        }
        node = node.previousSibling;
        if (!node || node.nodeType == 1) {
            break;
        }
        text.push(node.nodeValue);
        startContainer = node;
    }
    text.reverse();
    text = text.join("");
    if (!text) {
        return;
    }

    var pattern = this.wikiDetectTracLinkPattern;
    pattern.lastIndex = /[^ \t\r\n\f\v]*$/.exec(text).index;
    var match, tmp;
    for (tmp = pattern.exec(text); tmp; tmp = pattern.exec(text)) {
        match = tmp;
    }
    if (!match) {
        return;
    }

    var label = match[0];
    var link = this.normalizeTracLink(label);
    var id = this.generateDomId();
    var anchor = this.createAnchor(link, label, { id: id, "data-tracwysiwyg-autolink": "true" });
    var anonymous = this.contentDocument.createElement("div");
    anonymous.appendChild(anchor);
    var html = anonymous.innerHTML;

    node = endContainer;
    var startOffset = match.index;
    while (startContainer != node && startOffset >= startContainer.nodeValue.length) {
        startOffset -= startContainer.nodeValue.length;
        startContainer = startContainer.nextSibling;
    }
    var endOffset = startOffset + label.length;
    endContainer = startContainer;
    while (endContainer != node && endOffset >= endContainer.nodeValue.length) {
        endOffset -= endContainer.nodeValue.length;
        endContainer = endContainer.nextSibling;
    }
    this.selectRange(startContainer, startOffset, endContainer, endOffset);

    offset = text.length - match.index - label.length;
    if (offset == 0) {
        switch (event.keyCode) {
        case 0x20:  // SPACE
            this.insertHTML(html + "\u00a0");
            TracWysiwyg.stopEvent(event);
            return;
        case 0x0d:  // ENTER
            if (event.shiftKey) {
                if (window.opera || !anonymous.addEventListener) {
                    this.insertHTML(html + "<br>");
                    if (window.opera) {
                        anchor = this.contentDocument.getElementById(id);
                        node = anchor.parentNode;
                        offset = node.childNodes.length;
                        this.selectRange(node, offset, node, offset);
                    }
                    TracWysiwyg.stopEvent(event);
                    return;
                }
            }
            this.insertHTML(html);
            anchor = this.contentDocument.getElementById(id);
            node = event.shiftKey ? anchor.parentNode : anchor;
            offset = node.childNodes.length;
            this.selectRange(node, offset, node, offset);
            return;
        }
    }
    this.insertHTML(html);
    anchor = this.contentDocument.getElementById(id);
    node = anchor.nextSibling;
    if (!node) {
        node = anchor.parentNode;
        offset = node.childNodes.length;
    }
    this.selectRange(node, offset, node, offset);
};

prototype.formatParagraph = function() {
    if (this.selectionContainsTagName("table")) {
        return;
    }
    this.execCommand("formatblock", "<p>");
    this.selectionChanged();
};

prototype.formatHeaderBlock = function(name) {
    if (this.selectionContainsTagName("table")) {
        return;
    }
    this.execCommand("formatblock", "<" + name + ">");
    this.selectionChanged();
};

prototype.insertOrderedList = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    this.execCommand("insertorderedlist");
    this.selectionChanged();
};

prototype.insertUnorderedList = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    this.execCommand("insertunorderedlist");
    this.selectionChanged();
};

prototype.outdent = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    this.execCommand("outdent");
};

prototype.indent = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    this.execCommand("indent");
};

prototype.insertTable = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    var id = this.generateDomId();
    this.insertHTML(this.tableHTML(id, 2, 3));
    var element = this.contentDocument.getElementById(id)
    if (element) {
        this.selectNodeContents(element);
    }
    this.selectionChanged();
};

prototype._tableHTML = function(row, col) {
    var tr = "<tr>" + ((1 << col) - 1).toString(2).replace(/1/g, "<td></td>") + "</tr>";
    var html = [
        '<table class="wiki">', '<tbody>',
        ((1 << row) - 1).toString(2).replace(/1/g, tr),
        '</tbody>', '</table>' ];
    return html.join("");
};

prototype._getFocusForTable = function() {
    var hash = { node: null, cell: null, row: null, table: null };
    hash.node = this.getFocusNode();
    hash.cell = hash.node ? TracWysiwyg.getSelfOrAncestor(hash.node, /^t[dh]$/) : null;
    hash.row = hash.cell ? TracWysiwyg.getSelfOrAncestor(hash.cell, "tr") : null;
    hash.table = hash.row ? TracWysiwyg.getSelfOrAncestor(hash.row, "table") : null;
    return hash;
};

prototype.insertTableRow = function(after) {
    var focus = this._getFocusForTable();
    if (focus.table && focus.row) {
        var d = this.contentDocument;
        var cells = focus.row.getElementsByTagName("td");
        var row = focus.table.insertRow(focus.row.rowIndex + (after ? 1 : 0));
        for (var j = 0; j < cells.length; j++) {
            this.insertTableCell(row, 0);
        }
    }
};

prototype.insertTableColumn = function(after) {
    var focus = this._getFocusForTable();
    if (focus.table && focus.cell) {
        var d = this.contentDocument;
        var rows = focus.table.rows;
        var length = rows.length;
        var cellIndex = focus.cell.cellIndex + (after ? 1 : 0);
        for (var i = 0; i < length; i++) {
            var row = rows[i];
            this.insertTableCell(row, Math.min(cellIndex, row.cells.length));
        }
    }
};

prototype.deleteTableRow = function() {
    var focus = this._getFocusForTable();
    if (focus.table && focus.row) {
        focus.table.deleteRow(focus.row.rowIndex);
    }
};

prototype.deleteTableColumn = function() {
    var focus = this._getFocusForTable();
    if (focus.table && focus.cell) {
        var rows = focus.table.rows;
        var length = rows.length;
        var cellIndex = focus.cell.cellIndex;
        for (var i = 0; i < length; i++) {
            var row = rows[i];
            if (cellIndex < row.cells.length) {
                row.deleteCell(cellIndex);
            }
        }
    }
};

prototype.moveFocusInTable = function(forward) {
    var getSelfOrAncestor = TracWysiwyg.getSelfOrAncestor;
    var focus = this.getFocusNode();
    var element = getSelfOrAncestor(focus, /^(?:t[dhr]|table)$/);
    var target, table, rows, cells;
    switch (element.tagName.toLowerCase()) {
    case "td": case "th":
        focus = element;
        var row = getSelfOrAncestor(element, "tr");
        cells = row.cells;
        if (forward) {
            if (focus.cellIndex + 1 < cells.length) {
                target = cells[focus.cellIndex + 1];
            }
            else {
                table = getSelfOrAncestor(row, /^(?:tbody|table)$/);
                rows = table.rows;
                target = row.rowIndex + 1 < rows.length ? rows[row.rowIndex + 1].cells[0] : null;
            }
        }
        else {
            if (focus.cellIndex > 0) {
                target = cells[focus.cellIndex - 1];
            }
            else {
                table = getSelfOrAncestor(row, /^(?:tbody|table)$/);
                rows = table.rows;
                if (row.rowIndex > 0) {
                    cells = rows[row.rowIndex - 1].cells;
                    target = cells[cells.length - 1];
                }
                else {
                    target = null;
                }
            }
        }
        break;
    case "tr":
        cells = element.cells;
        target = cells[forward ? 0 : cells.length - 1];
        break;
    case "tbody": case "table":
        rows = element.rows;
        cells = rows[forward ? 0 : rows.length - 1].cells;
        target = cells[forward ? 0 : cells.length - 1];
        break;
    }
    if (target) {
        this.selectNodeContents(target);
    }
    else if (table) {
        table = getSelfOrAncestor(table, "table");
        var parent = table.parentNode;
        var elements = parent.childNodes;
        var length = elements.length;
        for (var offset = 0; offset < length; offset++) {
            if (table == elements[offset]) {
                if (forward) {
                    offset++;
                }
                this.selectRange(parent, offset, parent, offset);
            }
        }
    }
};

prototype.formatCodeBlock = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    var text = this.getSelectionText();
    if (!text) {
        var node = this.getFocusNode();
        while (node.nodeType == 3) {
            node = node.parentNode;
        }
        text = TracWysiwyg.getTextContent(node);
        this.selectNode(node);
    }

    var fragment = this.getSelectionFragment();
    text = this.domToWikitext(fragment, { formatCodeBlock: true }).replace(/\s+$/, "");

    var d = this.contentDocument;
    var anonymous = d.createElement("div");
    var pre = d.createElement("pre");
    pre.className = "wiki";
    anonymous.appendChild(pre);
    if (text) {
        pre.appendChild(d.createTextNode(text));
    }

    this.insertHTML(anonymous.innerHTML);
    this.selectionChanged();
};

prototype.formatQuoteBlock = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    var d = this.contentDocument;
    var anonymous = d.createElement("div");

    var container = d.createElement("blockquote");
    container.className = "citation";
    var fragment = this.getSelectionFragment();
    var childNodes = fragment.childNodes;
    for (var i = childNodes.length - 1; i >= 0; i--) {
        var child = childNodes[i];
        var text = null;
        switch (child.nodeType) {
        case 1:
            if (child.tagName.toLowerCase() != "blockquote" || child.className != "citation") {
                text = TracWysiwyg.getTextContent(child);
            }
            break;
        case 3:
            text = child.nodeValue;
            break;
        default:
            continue;
        }
        if (text !== null) {
            if (!text) {
                continue;
            }
            var tmp = d.createElement("p");
            tmp.appendChild(d.createTextNode(text));
            child = tmp;
        }
        container.insertBefore(child, container.firstChild);
    }
    if (container.childNodes.length == 0) {
        container.appendChild(d.createElement("p"));
    }
    anonymous.appendChild(container);

    this.insertHTML(anonymous.innerHTML);
    this.selectionChanged();
};

prototype.insertHorizontalRule = function() {
    if (this.selectionContainsTagName("table") || this.selectionContainsTagName("pre")) {
        return;
    }
    if (!this.execCommand("inserthorizontalrule")) {
        this.insertHTML("<hr />");
    }
    this.selectionChanged();
};

prototype.createLink = function() {
    if (this.selectionContainsTagName("pre")) {
        return;
    }

    var focus = this.getFocusNode();
    var anchor = TracWysiwyg.getSelfOrAncestor(focus, "a");
    var expand = anchor || TracWysiwyg.getSelfOrAncestor(focus, "tt");
    var currLink;
    if (anchor) {
        var attrs;
        var autolink = anchor.getAttribute("data-tracwysiwyg-autolink");
        if (autolink === null) {
            attrs = TracWysiwyg.unserializeFromHref(anchor.href);
            autolink = attrs["data-tracwysiwyg-autolink"];
        }
        if (autolink == "true") {
            var pattern = this.wikiDetectTracLinkPattern;
            pattern.lastIndex = 0;
            var label = TracWysiwyg.getTextContent(anchor);
            var match = pattern.exec(label);
            if (match && match.index == 0 && match[0].length == label.length) {
                currLink = this.normalizeTracLink(label);
            }
        }
        if (!currLink) {
            currLink = anchor.getAttribute("data-tracwysiwyg-link") || attrs["data-tracwysiwyg-link"]
                || anchor.href;
        }
    }
    else {
        currLink = "";
    }
    if (expand) {
        this.selectNodeContents(expand);
    }
    var text = this.getSelectionText() || "";
    var newLink = (prompt(text ? "Enter TracLink:" : "Insert TracLink:", currLink) || "").replace(/^\s+|\s+$/g, "");
    if (newLink && newLink != currLink) {
        text = text || newLink;
        newLink = this.normalizeTracLink(newLink);
        var id = this.generateDomId();
        var d = this.contentDocument;
        var anonymous = d.createElement("div");
        anchor = this.createAnchor(newLink, text, { id: id });
        anonymous.appendChild(anchor);
        this.insertHTML(anonymous.innerHTML);
        anchor = d.getElementById(id);
        if (anchor) {
            this.selectNodeContents(anchor);
        }
    }
    this.selectionChanged();
};

prototype.createAnchor = function(link, label, attrs) {
    var d = this.contentDocument;
    var anchor = d.createElement("a");
    var href = {};
    for (var name in attrs) {
        var value = attrs[name];
        href[name] = value;
        anchor.setAttribute(name, value);
    }
    href["data-tracwysiwyg-link"] = link;
    anchor.href = TracWysiwyg.serializeToHref(href);
    anchor.title = link;
    anchor.setAttribute("data-tracwysiwyg-link", link);
    anchor.setAttribute("onclick", "return false;");
    anchor.appendChild(d.createTextNode(label));
    return anchor;
};
prototype.collectChildNodes = function(dest, source) {
    var childNodes = source.childNodes;
    for (var i = childNodes.length - 1; i >= 0; i--) {
        dest.insertBefore(childNodes[i], dest.firstChild);
    }
};

prototype.generateDomId = function() {
    var d = this.contentDocument;
    for ( ; ; ) {
        var id = "tmp-" + (new Date().valueOf().toString(36));
        if (!d.getElementById(id)) {
            return id;
        }
    }
};

prototype.selectionChanged = function() {
    var status = {
        strong: false, em: false, underline: false, strike: false, sub: false,
        sup: false, monospace: false, paragraph: false, heading1: false,
        heading2: false, heading3: false, heading4: false, heading5: false,
        heading6: false, link: false, ol: false, ul: false, outdent: false,
        indent: false, table: false, code: false, quote: false, hr: false,
        br: false };
    var tagNameToKey = {
        b: "strong", i: "em", u: "underline", del: "strike", tt: "monospace",
        p: "paragraph", h1: "heading1", h2: "heading2", h3: "heading3",
        h4: "heading4", h5: "heading5", h6: "heading6", a: "link", pre: "code",
        blockquote: "quote" };
    var position = this.getSelectionPosition();

    var node;
    if (position.start) {
        node = position.start == position.end ? position.start.firstChild : position.start.nextSibling;
        node = node || position.start;
    }
    else {
        node = null;
    }
    while (node) {
        if (node.nodeType == 1) {
            var name = node.tagName.toLowerCase();
            if (name in tagNameToKey) {
                name = tagNameToKey[name];
            }
            status[name] = true;
        }
        node = node.parentNode;
    }

    var toolbarButtons = this.toolbarButtons;
    for (var name in status) {
        var button = toolbarButtons[name];
        if (button) {
            var parent = button.parentNode;
            parent.className = (parent.className || "").replace(/ *\bselected\b|$/, status[name] ? " selected" : "");
        }
    }

    var styles = [ "quote", "paragraph", "code", "heading1",
        "heading2", "heading3", "heading4", "heading5", "heading6" ];
    var styleButton = toolbarButtons["style"];
    var styleButtonClass = "wysiwyg-menu-style";
    for (var i = 0; i < styles.length; i++) {
        var name = styles[i];
        if (status[name]) {
            styleButtonClass = "wysiwyg-menu-" + name;
            break;
        }
    }
    styleButton.parentNode.className = styleButtonClass;
};

(function() {
    var _linkScheme = "[a-zA-Z][a-zA-Z0-9+-.]*";
    // cf. WikiSystem.XML_NAME, http://www.w3.org/TR/REC-xml/#id
    var _xmlName =
        "[:_A-Za-z\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u02ff\u0370-\u037d" +
        "\u037f-\u1fff\u200c-\u200d\u2070-\u218f\u2c00-\u2fef\u3001-\ud7ff" +
        "\uf900-\ufdcf\ufdf0-\ufffd]" +
        "(?:[-:_.A-Za-z0-9\u00b7\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u037d" +
        "\u037f-\u1fff\u200c-\u200d\u203f-\u2040\u2070-\u218f\u2c00-\u2fef" +
        "\u3001-\ud7ff\uf900-\ufdcf\ufdf0-\ufffd]*" +
        "[-_A-Za-z0-9\u00b7\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u037d" +
        "\u037f-\u1fff\u200c-\u200d\u203f-\u2040\u2070-\u218f\u2c00-\u2fef" +
        "\u3001-\ud7ff\uf900-\ufdcf\ufdf0-\ufffd])?"
    var _Lu =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ\u00c0\u00c1\u00c2\u00c3\u00c4\u00c5" +
        "\u00c6\u00c7\u00c8\u00c9\u00ca\u00cb\u00cc\u00cd\u00ce\u00cf\u00d0" +
        "\u00d1\u00d2\u00d3\u00d4\u00d5\u00d6\u00d8\u00d9\u00da\u00db\u00dc" +
        "\u00dd\u00de\u0100\u0102\u0104\u0106\u0108\u010a\u010c\u010e\u0110" +
        "\u0112\u0114\u0116\u0118\u011a\u011c\u011e\u0120\u0122\u0124\u0126" +
        "\u0128\u012a\u012c\u012e\u0130\u0132\u0134\u0136\u0139\u013b\u013d" +
        "\u013f\u0141\u0143\u0145\u0147\u014a\u014c\u014e\u0150\u0152\u0154" +
        "\u0156\u0158\u015a\u015c\u015e\u0160\u0162\u0164\u0166\u0168\u016a" +
        "\u016c\u016e\u0170\u0172\u0174\u0176\u0178\u0179\u017b\u017d\u0181" +
        "\u0182\u0184\u0186\u0187\u0189\u018a\u018b\u018e\u018f\u0190\u0191" +
        "\u0193\u0194\u0196\u0197\u0198\u019c\u019d\u019f\u01a0\u01a2\u01a4" +
        "\u01a6\u01a7\u01a9\u01ac\u01ae\u01af\u01b1\u01b2\u01b3\u01b5\u01b7" +
        "\u01b8\u01bc\u01c4\u01c7\u01ca\u01cd\u01cf\u01d1\u01d3\u01d5\u01d7" +
        "\u01d9\u01db\u01de\u01e0\u01e2\u01e4\u01e6\u01e8\u01ea\u01ec\u01ee" +
        "\u01f1\u01f4\u01f6\u01f7\u01f8\u01fa\u01fc\u01fe\u0200\u0202\u0204" +
        "\u0206\u0208\u020a\u020c\u020e\u0210\u0212\u0214\u0216\u0218\u021a" +
        "\u021c\u021e\u0220\u0222\u0224\u0226\u0228\u022a\u022c\u022e\u0230" +
        "\u0232\u023a\u023b\u023d\u023e\u0241\u0243\u0244\u0245\u0246\u0248" +
        "\u024a\u024c\u024e\u0370\u0372\u0376\u0386\u0388\u0389\u038a\u038c" +
        "\u038e\u038f\u0391\u0392\u0393\u0394\u0395\u0396\u0397\u0398\u0399" +
        "\u039a\u039b\u039c\u039d\u039e\u039f\u03a0\u03a1\u03a3\u03a4\u03a5" +
        "\u03a6\u03a7\u03a8\u03a9\u03aa\u03ab\u03cf\u03d2\u03d3\u03d4\u03d8" +
        "\u03da\u03dc\u03de\u03e0\u03e2\u03e4\u03e6\u03e8\u03ea\u03ec\u03ee" +
        "\u03f4\u03f7\u03f9\u03fa\u03fd\u03fe\u03ff\u0400\u0401\u0402\u0403" +
        "\u0404\u0405\u0406\u0407\u0408\u0409\u040a\u040b\u040c\u040d\u040e" +
        "\u040f\u0410\u0411\u0412\u0413\u0414\u0415\u0416\u0417\u0418\u0419" +
        "\u041a\u041b\u041c\u041d\u041e\u041f\u0420\u0421\u0422\u0423\u0424" +
        "\u0425\u0426\u0427\u0428\u0429\u042a\u042b\u042c\u042d\u042e\u042f" +
        "\u0460\u0462\u0464\u0466\u0468\u046a\u046c\u046e\u0470\u0472\u0474" +
        "\u0476\u0478\u047a\u047c\u047e\u0480\u048a\u048c\u048e\u0490\u0492" +
        "\u0494\u0496\u0498\u049a\u049c\u049e\u04a0\u04a2\u04a4\u04a6\u04a8" +
        "\u04aa\u04ac\u04ae\u04b0\u04b2\u04b4\u04b6\u04b8\u04ba\u04bc\u04be" +
        "\u04c0\u04c1\u04c3\u04c5\u04c7\u04c9\u04cb\u04cd\u04d0\u04d2\u04d4" +
        "\u04d6\u04d8\u04da\u04dc\u04de\u04e0\u04e2\u04e4\u04e6\u04e8\u04ea" +
        "\u04ec\u04ee\u04f0\u04f2\u04f4\u04f6\u04f8\u04fa\u04fc\u04fe\u0500" +
        "\u0502\u0504\u0506\u0508\u050a\u050c\u050e\u0510\u0512\u0514\u0516" +
        "\u0518\u051a\u051c\u051e\u0520\u0522\u0524\u0531\u0532\u0533\u0534" +
        "\u0535\u0536\u0537\u0538\u0539\u053a\u053b\u053c\u053d\u053e\u053f" +
        "\u0540\u0541\u0542\u0543\u0544\u0545\u0546\u0547\u0548\u0549\u054a" +
        "\u054b\u054c\u054d\u054e\u054f\u0550\u0551\u0552\u0553\u0554\u0555" +
        "\u0556\u10a0\u10a1\u10a2\u10a3\u10a4\u10a5\u10a6\u10a7\u10a8\u10a9" +
        "\u10aa\u10ab\u10ac\u10ad\u10ae\u10af\u10b0\u10b1\u10b2\u10b3\u10b4" +
        "\u10b5\u10b6\u10b7\u10b8\u10b9\u10ba\u10bb\u10bc\u10bd\u10be\u10bf" +
        "\u10c0\u10c1\u10c2\u10c3\u10c4\u10c5\u1e00\u1e02\u1e04\u1e06\u1e08" +
        "\u1e0a\u1e0c\u1e0e\u1e10\u1e12\u1e14\u1e16\u1e18\u1e1a\u1e1c\u1e1e" +
        "\u1e20\u1e22\u1e24\u1e26\u1e28\u1e2a\u1e2c\u1e2e\u1e30\u1e32\u1e34" +
        "\u1e36\u1e38\u1e3a\u1e3c\u1e3e\u1e40\u1e42\u1e44\u1e46\u1e48\u1e4a" +
        "\u1e4c\u1e4e\u1e50\u1e52\u1e54\u1e56\u1e58\u1e5a\u1e5c\u1e5e\u1e60" +
        "\u1e62\u1e64\u1e66\u1e68\u1e6a\u1e6c\u1e6e\u1e70\u1e72\u1e74\u1e76" +
        "\u1e78\u1e7a\u1e7c\u1e7e\u1e80\u1e82\u1e84\u1e86\u1e88\u1e8a\u1e8c" +
        "\u1e8e\u1e90\u1e92\u1e94\u1e9e\u1ea0\u1ea2\u1ea4\u1ea6\u1ea8\u1eaa" +
        "\u1eac\u1eae\u1eb0\u1eb2\u1eb4\u1eb6\u1eb8\u1eba\u1ebc\u1ebe\u1ec0" +
        "\u1ec2\u1ec4\u1ec6\u1ec8\u1eca\u1ecc\u1ece\u1ed0\u1ed2\u1ed4\u1ed6" +
        "\u1ed8\u1eda\u1edc\u1ede\u1ee0\u1ee2\u1ee4\u1ee6\u1ee8\u1eea\u1eec" +
        "\u1eee\u1ef0\u1ef2\u1ef4\u1ef6\u1ef8\u1efa\u1efc\u1efe\u1f08\u1f09" +
        "\u1f0a\u1f0b\u1f0c\u1f0d\u1f0e\u1f0f\u1f18\u1f19\u1f1a\u1f1b\u1f1c" +
        "\u1f1d\u1f28\u1f29\u1f2a\u1f2b\u1f2c\u1f2d\u1f2e\u1f2f\u1f38\u1f39" +
        "\u1f3a\u1f3b\u1f3c\u1f3d\u1f3e\u1f3f\u1f48\u1f49\u1f4a\u1f4b\u1f4c" +
        "\u1f4d\u1f59\u1f5b\u1f5d\u1f5f\u1f68\u1f69\u1f6a\u1f6b\u1f6c\u1f6d" +
        "\u1f6e\u1f6f\u1fb8\u1fb9\u1fba\u1fbb\u1fc8\u1fc9\u1fca\u1fcb\u1fd8" +
        "\u1fd9\u1fda\u1fdb\u1fe8\u1fe9\u1fea\u1feb\u1fec\u1ff8\u1ff9\u1ffa" +
        "\u1ffb\u2102\u2107\u210b\u210c\u210d\u2110\u2111\u2112\u2115\u2119" +
        "\u211a\u211b\u211c\u211d\u2124\u2126\u2128\u212a\u212b\u212c\u212d" +
        "\u2130\u2131\u2132\u2133\u213e\u213f\u2145\u2183\u2c00\u2c01\u2c02" +
        "\u2c03\u2c04\u2c05\u2c06\u2c07\u2c08\u2c09\u2c0a\u2c0b\u2c0c\u2c0d" +
        "\u2c0e\u2c0f\u2c10\u2c11\u2c12\u2c13\u2c14\u2c15\u2c16\u2c17\u2c18" +
        "\u2c19\u2c1a\u2c1b\u2c1c\u2c1d\u2c1e\u2c1f\u2c20\u2c21\u2c22\u2c23" +
        "\u2c24\u2c25\u2c26\u2c27\u2c28\u2c29\u2c2a\u2c2b\u2c2c\u2c2d\u2c2e" +
        "\u2c60\u2c62\u2c63\u2c64\u2c67\u2c69\u2c6b\u2c6d\u2c6e\u2c6f\u2c70" +
        "\u2c72\u2c75\u2c7e\u2c7f\u2c80\u2c82\u2c84\u2c86\u2c88\u2c8a\u2c8c" +
        "\u2c8e\u2c90\u2c92\u2c94\u2c96\u2c98\u2c9a\u2c9c\u2c9e\u2ca0\u2ca2" +
        "\u2ca4\u2ca6\u2ca8\u2caa\u2cac\u2cae\u2cb0\u2cb2\u2cb4\u2cb6\u2cb8" +
        "\u2cba\u2cbc\u2cbe\u2cc0\u2cc2\u2cc4\u2cc6\u2cc8\u2cca\u2ccc\u2cce" +
        "\u2cd0\u2cd2\u2cd4\u2cd6\u2cd8\u2cda\u2cdc\u2cde\u2ce0\u2ce2\u2ceb" +
        "\u2ced\ua640\ua642\ua644\ua646\ua648\ua64a\ua64c\ua64e\ua650\ua652" +
        "\ua654\ua656\ua658\ua65a\ua65c\ua65e\ua662\ua664\ua666\ua668\ua66a" +
        "\ua66c\ua680\ua682\ua684\ua686\ua688\ua68a\ua68c\ua68e\ua690\ua692" +
        "\ua694\ua696\ua722\ua724\ua726\ua728\ua72a\ua72c\ua72e\ua732\ua734" +
        "\ua736\ua738\ua73a\ua73c\ua73e\ua740\ua742\ua744\ua746\ua748\ua74a" +
        "\ua74c\ua74e\ua750\ua752\ua754\ua756\ua758\ua75a\ua75c\ua75e\ua760" +
        "\ua762\ua764\ua766\ua768\ua76a\ua76c\ua76e\ua779\ua77b\ua77d\ua77e" +
        "\ua780\ua782\ua784\ua786\ua78b\uff21\uff22\uff23\uff24\uff25\uff26" +
        "\uff27\uff28\uff29\uff2a\uff2b\uff2c\uff2d\uff2e\uff2f\uff30\uff31" +
        "\uff32\uff33\uff34\uff35\uff36\uff37\uff38\uff39\uff3a";
    var _Ll =
        "abcdefghijklmnopqrstuvwxyz\u00aa\u00b5\u00ba\u00df\u00e0\u00e1" +
        "\u00e2\u00e3\u00e4\u00e5\u00e6\u00e7\u00e8\u00e9\u00ea\u00eb\u00ec" +
        "\u00ed\u00ee\u00ef\u00f0\u00f1\u00f2\u00f3\u00f4\u00f5\u00f6\u00f8" +
        "\u00f9\u00fa\u00fb\u00fc\u00fd\u00fe\u00ff\u0101\u0103\u0105\u0107" +
        "\u0109\u010b\u010d\u010f\u0111\u0113\u0115\u0117\u0119\u011b\u011d" +
        "\u011f\u0121\u0123\u0125\u0127\u0129\u012b\u012d\u012f\u0131\u0133" +
        "\u0135\u0137\u0138\u013a\u013c\u013e\u0140\u0142\u0144\u0146\u0148" +
        "\u0149\u014b\u014d\u014f\u0151\u0153\u0155\u0157\u0159\u015b\u015d" +
        "\u015f\u0161\u0163\u0165\u0167\u0169\u016b\u016d\u016f\u0171\u0173" +
        "\u0175\u0177\u017a\u017c\u017e\u017f\u0180\u0183\u0185\u0188\u018c" +
        "\u018d\u0192\u0195\u0199\u019a\u019b\u019e\u01a1\u01a3\u01a5\u01a8" +
        "\u01aa\u01ab\u01ad\u01b0\u01b4\u01b6\u01b9\u01ba\u01bd\u01be\u01bf" +
        "\u01c6\u01c9\u01cc\u01ce\u01d0\u01d2\u01d4\u01d6\u01d8\u01da\u01dc" +
        "\u01dd\u01df\u01e1\u01e3\u01e5\u01e7\u01e9\u01eb\u01ed\u01ef\u01f0" +
        "\u01f3\u01f5\u01f9\u01fb\u01fd\u01ff\u0201\u0203\u0205\u0207\u0209" +
        "\u020b\u020d\u020f\u0211\u0213\u0215\u0217\u0219\u021b\u021d\u021f" +
        "\u0221\u0223\u0225\u0227\u0229\u022b\u022d\u022f\u0231\u0233\u0234" +
        "\u0235\u0236\u0237\u0238\u0239\u023c\u023f\u0240\u0242\u0247\u0249" +
        "\u024b\u024d\u024f\u0250\u0251\u0252\u0253\u0254\u0255\u0256\u0257" +
        "\u0258\u0259\u025a\u025b\u025c\u025d\u025e\u025f\u0260\u0261\u0262" +
        "\u0263\u0264\u0265\u0266\u0267\u0268\u0269\u026a\u026b\u026c\u026d" +
        "\u026e\u026f\u0270\u0271\u0272\u0273\u0274\u0275\u0276\u0277\u0278" +
        "\u0279\u027a\u027b\u027c\u027d\u027e\u027f\u0280\u0281\u0282\u0283" +
        "\u0284\u0285\u0286\u0287\u0288\u0289\u028a\u028b\u028c\u028d\u028e" +
        "\u028f\u0290\u0291\u0292\u0293\u0295\u0296\u0297\u0298\u0299\u029a" +
        "\u029b\u029c\u029d\u029e\u029f\u02a0\u02a1\u02a2\u02a3\u02a4\u02a5" +
        "\u02a6\u02a7\u02a8\u02a9\u02aa\u02ab\u02ac\u02ad\u02ae\u02af\u0371" +
        "\u0373\u0377\u037b\u037c\u037d\u0390\u03ac\u03ad\u03ae\u03af\u03b0" +
        "\u03b1\u03b2\u03b3\u03b4\u03b5\u03b6\u03b7\u03b8\u03b9\u03ba\u03bb" +
        "\u03bc\u03bd\u03be\u03bf\u03c0\u03c1\u03c2\u03c3\u03c4\u03c5\u03c6" +
        "\u03c7\u03c8\u03c9\u03ca\u03cb\u03cc\u03cd\u03ce\u03d0\u03d1\u03d5" +
        "\u03d6\u03d7\u03d9\u03db\u03dd\u03df\u03e1\u03e3\u03e5\u03e7\u03e9" +
        "\u03eb\u03ed\u03ef\u03f0\u03f1\u03f2\u03f3\u03f5\u03f8\u03fb\u03fc" +
        "\u0430\u0431\u0432\u0433\u0434\u0435\u0436\u0437\u0438\u0439\u043a" +
        "\u043b\u043c\u043d\u043e\u043f\u0440\u0441\u0442\u0443\u0444\u0445" +
        "\u0446\u0447\u0448\u0449\u044a\u044b\u044c\u044d\u044e\u044f\u0450" +
        "\u0451\u0452\u0453\u0454\u0455\u0456\u0457\u0458\u0459\u045a\u045b" +
        "\u045c\u045d\u045e\u045f\u0461\u0463\u0465\u0467\u0469\u046b\u046d" +
        "\u046f\u0471\u0473\u0475\u0477\u0479\u047b\u047d\u047f\u0481\u048b" +
        "\u048d\u048f\u0491\u0493\u0495\u0497\u0499\u049b\u049d\u049f\u04a1" +
        "\u04a3\u04a5\u04a7\u04a9\u04ab\u04ad\u04af\u04b1\u04b3\u04b5\u04b7" +
        "\u04b9\u04bb\u04bd\u04bf\u04c2\u04c4\u04c6\u04c8\u04ca\u04cc\u04ce" +
        "\u04cf\u04d1\u04d3\u04d5\u04d7\u04d9\u04db\u04dd\u04df\u04e1\u04e3" +
        "\u04e5\u04e7\u04e9\u04eb\u04ed\u04ef\u04f1\u04f3\u04f5\u04f7\u04f9" +
        "\u04fb\u04fd\u04ff\u0501\u0503\u0505\u0507\u0509\u050b\u050d\u050f" +
        "\u0511\u0513\u0515\u0517\u0519\u051b\u051d\u051f\u0521\u0523\u0525" +
        "\u0561\u0562\u0563\u0564\u0565\u0566\u0567\u0568\u0569\u056a\u056b" +
        "\u056c\u056d\u056e\u056f\u0570\u0571\u0572\u0573\u0574\u0575\u0576" +
        "\u0577\u0578\u0579\u057a\u057b\u057c\u057d\u057e\u057f\u0580\u0581" +
        "\u0582\u0583\u0584\u0585\u0586\u0587\u1d00\u1d01\u1d02\u1d03\u1d04" +
        "\u1d05\u1d06\u1d07\u1d08\u1d09\u1d0a\u1d0b\u1d0c\u1d0d\u1d0e\u1d0f" +
        "\u1d10\u1d11\u1d12\u1d13\u1d14\u1d15\u1d16\u1d17\u1d18\u1d19\u1d1a" +
        "\u1d1b\u1d1c\u1d1d\u1d1e\u1d1f\u1d20\u1d21\u1d22\u1d23\u1d24\u1d25" +
        "\u1d26\u1d27\u1d28\u1d29\u1d2a\u1d2b\u1d62\u1d63\u1d64\u1d65\u1d66" +
        "\u1d67\u1d68\u1d69\u1d6a\u1d6b\u1d6c\u1d6d\u1d6e\u1d6f\u1d70\u1d71" +
        "\u1d72\u1d73\u1d74\u1d75\u1d76\u1d77\u1d79\u1d7a\u1d7b\u1d7c\u1d7d" +
        "\u1d7e\u1d7f\u1d80\u1d81\u1d82\u1d83\u1d84\u1d85\u1d86\u1d87\u1d88" +
        "\u1d89\u1d8a\u1d8b\u1d8c\u1d8d\u1d8e\u1d8f\u1d90\u1d91\u1d92\u1d93" +
        "\u1d94\u1d95\u1d96\u1d97\u1d98\u1d99\u1d9a\u1e01\u1e03\u1e05\u1e07" +
        "\u1e09\u1e0b\u1e0d\u1e0f\u1e11\u1e13\u1e15\u1e17\u1e19\u1e1b\u1e1d" +
        "\u1e1f\u1e21\u1e23\u1e25\u1e27\u1e29\u1e2b\u1e2d\u1e2f\u1e31\u1e33" +
        "\u1e35\u1e37\u1e39\u1e3b\u1e3d\u1e3f\u1e41\u1e43\u1e45\u1e47\u1e49" +
        "\u1e4b\u1e4d\u1e4f\u1e51\u1e53\u1e55\u1e57\u1e59\u1e5b\u1e5d\u1e5f" +
        "\u1e61\u1e63\u1e65\u1e67\u1e69\u1e6b\u1e6d\u1e6f\u1e71\u1e73\u1e75" +
        "\u1e77\u1e79\u1e7b\u1e7d\u1e7f\u1e81\u1e83\u1e85\u1e87\u1e89\u1e8b" +
        "\u1e8d\u1e8f\u1e91\u1e93\u1e95\u1e96\u1e97\u1e98\u1e99\u1e9a\u1e9b" +
        "\u1e9c\u1e9d\u1e9f\u1ea1\u1ea3\u1ea5\u1ea7\u1ea9\u1eab\u1ead\u1eaf" +
        "\u1eb1\u1eb3\u1eb5\u1eb7\u1eb9\u1ebb\u1ebd\u1ebf\u1ec1\u1ec3\u1ec5" +
        "\u1ec7\u1ec9\u1ecb\u1ecd\u1ecf\u1ed1\u1ed3\u1ed5\u1ed7\u1ed9\u1edb" +
        "\u1edd\u1edf\u1ee1\u1ee3\u1ee5\u1ee7\u1ee9\u1eeb\u1eed\u1eef\u1ef1" +
        "\u1ef3\u1ef5\u1ef7\u1ef9\u1efb\u1efd\u1eff\u1f00\u1f01\u1f02\u1f03" +
        "\u1f04\u1f05\u1f06\u1f07\u1f10\u1f11\u1f12\u1f13\u1f14\u1f15\u1f20" +
        "\u1f21\u1f22\u1f23\u1f24\u1f25\u1f26\u1f27\u1f30\u1f31\u1f32\u1f33" +
        "\u1f34\u1f35\u1f36\u1f37\u1f40\u1f41\u1f42\u1f43\u1f44\u1f45\u1f50" +
        "\u1f51\u1f52\u1f53\u1f54\u1f55\u1f56\u1f57\u1f60\u1f61\u1f62\u1f63" +
        "\u1f64\u1f65\u1f66\u1f67\u1f70\u1f71\u1f72\u1f73\u1f74\u1f75\u1f76" +
        "\u1f77\u1f78\u1f79\u1f7a\u1f7b\u1f7c\u1f7d\u1f80\u1f81\u1f82\u1f83" +
        "\u1f84\u1f85\u1f86\u1f87\u1f90\u1f91\u1f92\u1f93\u1f94\u1f95\u1f96" +
        "\u1f97\u1fa0\u1fa1\u1fa2\u1fa3\u1fa4\u1fa5\u1fa6\u1fa7\u1fb0\u1fb1" +
        "\u1fb2\u1fb3\u1fb4\u1fb6\u1fb7\u1fbe\u1fc2\u1fc3\u1fc4\u1fc6\u1fc7" +
        "\u1fd0\u1fd1\u1fd2\u1fd3\u1fd6\u1fd7\u1fe0\u1fe1\u1fe2\u1fe3\u1fe4" +
        "\u1fe5\u1fe6\u1fe7\u1ff2\u1ff3\u1ff4\u1ff6\u1ff7\u210a\u210e\u210f" +
        "\u2113\u212f\u2134\u2139\u213c\u213d\u2146\u2147\u2148\u2149\u214e" +
        "\u2184\u2c30\u2c31\u2c32\u2c33\u2c34\u2c35\u2c36\u2c37\u2c38\u2c39" +
        "\u2c3a\u2c3b\u2c3c\u2c3d\u2c3e\u2c3f\u2c40\u2c41\u2c42\u2c43\u2c44" +
        "\u2c45\u2c46\u2c47\u2c48\u2c49\u2c4a\u2c4b\u2c4c\u2c4d\u2c4e\u2c4f" +
        "\u2c50\u2c51\u2c52\u2c53\u2c54\u2c55\u2c56\u2c57\u2c58\u2c59\u2c5a" +
        "\u2c5b\u2c5c\u2c5d\u2c5e\u2c61\u2c65\u2c66\u2c68\u2c6a\u2c6c\u2c71" +
        "\u2c73\u2c74\u2c76\u2c77\u2c78\u2c79\u2c7a\u2c7b\u2c7c\u2c81\u2c83" +
        "\u2c85\u2c87\u2c89\u2c8b\u2c8d\u2c8f\u2c91\u2c93\u2c95\u2c97\u2c99" +
        "\u2c9b\u2c9d\u2c9f\u2ca1\u2ca3\u2ca5\u2ca7\u2ca9\u2cab\u2cad\u2caf" +
        "\u2cb1\u2cb3\u2cb5\u2cb7\u2cb9\u2cbb\u2cbd\u2cbf\u2cc1\u2cc3\u2cc5" +
        "\u2cc7\u2cc9\u2ccb\u2ccd\u2ccf\u2cd1\u2cd3\u2cd5\u2cd7\u2cd9\u2cdb" +
        "\u2cdd\u2cdf\u2ce1\u2ce3\u2ce4\u2cec\u2cee\u2d00\u2d01\u2d02\u2d03" +
        "\u2d04\u2d05\u2d06\u2d07\u2d08\u2d09\u2d0a\u2d0b\u2d0c\u2d0d\u2d0e" +
        "\u2d0f\u2d10\u2d11\u2d12\u2d13\u2d14\u2d15\u2d16\u2d17\u2d18\u2d19" +
        "\u2d1a\u2d1b\u2d1c\u2d1d\u2d1e\u2d1f\u2d20\u2d21\u2d22\u2d23\u2d24" +
        "\u2d25\ua641\ua643\ua645\ua647\ua649\ua64b\ua64d\ua64f\ua651\ua653" +
        "\ua655\ua657\ua659\ua65b\ua65d\ua65f\ua663\ua665\ua667\ua669\ua66b" +
        "\ua66d\ua681\ua683\ua685\ua687\ua689\ua68b\ua68d\ua68f\ua691\ua693" +
        "\ua695\ua697\ua723\ua725\ua727\ua729\ua72b\ua72d\ua72f\ua730\ua731" +
        "\ua733\ua735\ua737\ua739\ua73b\ua73d\ua73f\ua741\ua743\ua745\ua747" +
        "\ua749\ua74b\ua74d\ua74f\ua751\ua753\ua755\ua757\ua759\ua75b\ua75d" +
        "\ua75f\ua761\ua763\ua765\ua767\ua769\ua76b\ua76d\ua76f\ua771\ua772" +
        "\ua773\ua774\ua775\ua776\ua777\ua778\ua77a\ua77c\ua77f\ua781\ua783" +
        "\ua785\ua787\ua78c\ufb00\ufb01\ufb02\ufb03\ufb04\ufb05\ufb06\ufb13" +
        "\ufb14\ufb15\ufb16\ufb17\uff41\uff42\uff43\uff44\uff45\uff46\uff47" +
        "\uff48\uff49\uff4a\uff4b\uff4c\uff4d\uff4e\uff4f\uff50\uff51\uff52" +
        "\uff53\uff54\uff55\uff56\uff57\uff58\uff59\uff5a";
    var _quotedString = "'[^']+'|" + '"[^"]+"';
    var _changesetId = "(?:\\d+|[a-fA-F\\d]{6,})";
    var _ticketLink = "#\\d+";
    var _reportLink = "\\{\\d+\\}";
    var _changesetPath = "/[^\\]]*";
    var _changesetLinkBracket = "\\[" + _changesetId + "(?:" + _changesetPath + ")?\\]";
    var _changesetLinkRev = "r" + _changesetId + "\\b(?!:" + _changesetId + ")";
    var _logLinkBracket = "\\[" + _changesetId + "[-:]" + _changesetId + "(?:" + _changesetPath + ")?\\]";
    var _logLinkRev = "r" + _changesetId + "[-:]" + _changesetId + "\\b";
    var _tracLink = _linkScheme + ":(?:" + _quotedString
        + "|[a-zA-Z0-9/?!#@](?:(?:\\|(?=[^| \\t\\r\\f\\v])|[^|<> \\t\\r\\f\\v])*[a-zA-Z0-9/=])?)";
    var _wikiPageName =
        "(?:[" + _Lu + "][" + _Ll + "]+/?){2,}" +
        "(?:@[0-9]+)?" +
        "(?:#" + _xmlName + ")?" +
        "(?=:(?:$|[ \\t\\r\\f\\v])|[^:\\w" + _Lu + _Ll + "]|[ \\t\\r\\f\\v]|$)";
    var wikiInlineRules = [];
    wikiInlineRules.push("!?'''''");        // 1. bolditalic
    wikiInlineRules.push("!?'''");          // 2. bold
    wikiInlineRules.push("!?''");           // 3. italic
    wikiInlineRules.push("!?__");           // 4. underline
    wikiInlineRules.push("!?~~");           // 5. strike
    wikiInlineRules.push("!?,,");           // 6. subscript
    wikiInlineRules.push("!?\\^");          // 7. superscript
    wikiInlineRules.push("!?\\{\\{\\{.*?\\}\\}\\}");  // 8. code block
    wikiInlineRules.push("!?`.*?`");        // 9. inline
    wikiInlineRules.push("[!&]?" + _ticketLink);    // 10. ticket
    wikiInlineRules.push("!?" + _reportLink);       // 11. report
                                            // 12. changeset
    wikiInlineRules.push("!?" + _changesetLinkBracket + "|(?:\\b|!)" + _changesetLinkRev);
                                            // 13. log
    wikiInlineRules.push("!?" + _logLinkBracket + "|(?:\\b|!)" + _logLinkRev);
    wikiInlineRules.push("!?" + _tracLink); // 14. wiki:TracLinks
    wikiInlineRules.push("!?\\[(?:"         // 15. [wiki:TracLinks label] or [/relative label]
        + "[/.#][^ \\t\\r\\f\\v[\\]]*|"
        + _linkScheme + ":(?:" + _quotedString + "|[^\\] \\t\\r\\f\\v]*)|"
        + _wikiPageName + "[ \\t\\r\\f\\v]+(?:" + _quotedString + "|[^\\]]+)"
        + ")(?:[ \\t\\r\\f\\v]+(?:" + _quotedString + "|[^\\]]+))?\\]");
                                            // 16. [[macro]]
    wikiInlineRules.push("!?\\[\\[(?:[\\w/+-]+\\??|\\?)(?:\\]\\]|\\(.*?\\)\\]\\])");
                                            // 17. WikiPageName
    wikiInlineRules.push("(?:\\b|!)" + _wikiPageName);
                                            // 18. ["internal free link"]
    wikiInlineRules.push("!?\\[(?:" + _quotedString + ")\\]");
                                            // 19. <wiki:Trac bracket links>
    wikiInlineRules.push("!?<@:[^>]+>".replace(/@/g, _linkScheme));
                                            // 20. [=#anchor label]
    wikiInlineRules.push("!?\\[=#" + _xmlName + "(?:[ \\t\\r\\f\\v]+[^\\]]*)?\\]");

    var wikiToDomInlineRules = wikiInlineRules.slice(0);
                                            // 1001. escaping double pipes
    wikiToDomInlineRules.push("!=?(?:\\|\\|)+(?:[ \\t\\r\\f\\v]*$|)");

    var wikiRules = wikiToDomInlineRules.slice(0);
    wikiRules.push("^(?: *>)+[ \\t\\r\\f\\v]*");    // -1. citation
                                            // -2. header
    wikiRules.push("^[ \\t\\r\\f\\v]*={1,6}[ \\t\\r\\f\\v]+.*?(?:#" + _xmlName + ")?[ \\t\\r\\f\\v]*$");
                                            // -3. list
    wikiRules.push("^[ \\t\\r\\f\\v]*(?:[-*]|[0-9]+\\.|[a-zA-Z]\\.|[ivxIVX]{1,5}\\.) ");
                                            // -4. definition
    wikiRules.push("^[ \\t\\r\\f\\v]+(?:`[^`]*`|\\{\\{\\{.*?\\}\\}\\}|[^`{:]|:[^:])+::(?:[ \\t\\r\\f\\v]+|$)");
    wikiRules.push("^[ \\t\\r\\f\\v]+(?=[^ \\t\\r\\f\\v])");    // -5. leading space
    wikiRules.push("=?(?:\\|\\|)+[ \\t\\r\\f\\v]*\\\\?$");      // -6. closing table row
    wikiRules.push("=?(?:\\|\\|)+=?");                  // -7. cell

    var domToWikiInlineRules = wikiInlineRules.slice(0);
    domToWikiInlineRules.push("!?=?(?:\\|\\|)+=?");     // cell

    var wikiSyntaxRules = [];
    wikiSyntaxRules.push(_ticketLink);
    wikiSyntaxRules.push(_reportLink);
    wikiSyntaxRules.push(_changesetLinkBracket);
    wikiSyntaxRules.push(_changesetLinkRev);
    wikiSyntaxRules.push(_logLinkBracket);
    wikiSyntaxRules.push(_logLinkRev);

    var wikiDetectTracLinkRules = [];
    wikiDetectTracLinkRules.push(_ticketLink);
    wikiDetectTracLinkRules.push(_reportLink);
    wikiDetectTracLinkRules.push(_changesetLinkBracket);
    wikiDetectTracLinkRules.push("\\b" + _changesetLinkRev);
    wikiDetectTracLinkRules.push(_logLinkBracket);
    wikiDetectTracLinkRules.push("\\b" + _logLinkRev);
    wikiDetectTracLinkRules.push(_tracLink);
    wikiDetectTracLinkRules.push("\\b" + _wikiPageName);

    var domToWikiInlinePattern = new RegExp("(?:" + domToWikiInlineRules.join("|") + ")", "g");
    var wikiRulesPattern = new RegExp("(?:(" + wikiRules.join(")|(") + "))", "g");
    var wikiSyntaxPattern = new RegExp("^(?:" + wikiSyntaxRules.join("|") + ")$");
    var wikiSyntaxLogPattern = new RegExp("^[\\[r]" + _changesetId + "[-:]");
    var wikiDetectTracLinkPattern = new RegExp("(?:" + wikiDetectTracLinkRules.join("|") + ")", "g");

    prototype._linkScheme = _linkScheme;
    prototype._quotedString = _quotedString;
    prototype._changesetId = _changesetId;
    prototype._tracLink = _tracLink;
    prototype._wikiPageName = _wikiPageName;
    prototype.wikiInlineRules = wikiInlineRules;
    prototype.wikiToDomInlineRules = wikiToDomInlineRules;
    prototype.xmlNamePattern = new RegExp("^" + _xmlName + "$");
    prototype.domToWikiInlinePattern = domToWikiInlinePattern;
    prototype.wikiRulesPattern = wikiRulesPattern;
    prototype.wikiSyntaxPattern = wikiSyntaxPattern;
    prototype.wikiSyntaxLogPattern = wikiSyntaxLogPattern;
    prototype.wikiDetectTracLinkPattern = wikiDetectTracLinkPattern;
})();

prototype.normalizeTracLink = function(link) {
    link = this.convertWikiSyntax(link);
    if (/^[\/.#]/.test(link)) {
        link = encode(link);
    }
    if (!/^[\w.+-]+:/.test(link)) {
        link = "wiki:" + link;
    }
    if (/^wiki:[^\"\']/.test(link) && /\s/.test(link)) {
        if (link.indexOf('"') === -1) {
            link = 'wiki:"' + link + '"';
        }
        else if (link.indexOf("'") === -1) {
            link = "wiki:'" + link + "'";
        }
        else {
            link = 'wiki:"' + link.replace(/"/g, "%22") + '"';
        }
    }
    return link;
};

prototype.convertWikiSyntax = function(link) {
    var match = this.wikiSyntaxPattern.exec(link);
    if (match) {
        switch (match[0].charCodeAt(0)) {
        case 0x7b:  // "{"
            link = "report:" + link.slice(1, -1);
            break;
        case 0x5b:  // "["
            link = (this.wikiSyntaxLogPattern.test(link) ? "log:@" : "changeset:") + link.slice(1, -1);
            break;
        case 0x23:  // #
            link = "ticket:" + link.substring(1);
            break;
        case 0x72:  // r
            link = (this.wikiSyntaxLogPattern.test(link) ? "log:@" : "changeset:") + link.substring(1);
            break;
        }
    }
    return link;
};

prototype.isInlineNode = function(node) {
    if (node) {
        switch (node.nodeType) {
        case 1:
            return (node.tagName.toLowerCase() in this.wikiInlineTags);
        case 3:
            return true;
        }
    }
    return false;
};

(function() {
    var blocks = {
        p: true, blockquote: true, div: true,
        li: true, ul: true, ol: true,
        dl: true, dt: true, dd: true,
        h1: true, h2: true, h3: true, h4: true, h5: true, h6: true,
        table: true, thead: true, tbody: true, tr: true, td: true, th: true };

    function generator(prop, blocks) {
        return function (node) {
            if (!node) {
                return false;
            }
            for ( ; ; ) {
                if (node[prop]) {
                    return false;
                }
                node = node.parentNode;
                if (!node) {
                    return true;
                }
                if (node.nodeType == 1 && node.tagName.toLowerCase() in blocks) {
                    return true;
                }
            }
            return false;
        };
    }

    prototype.isLastChildInBlockNode = generator("nextSibling", blocks);
    prototype.isFirstChildInBlockNode = generator("previousSibling", blocks);
})();

prototype.wikitextToFragment = function(wikitext, contentDocument, options) {
    options = options || {};
    var escapeNewlines = !!options.escapeNewlines;

    var getSelfOrAncestor = TracWysiwyg.getSelfOrAncestor;
    var _linkScheme = this._linkScheme;
    var _quotedString = this._quotedString;
    var wikiInlineRulesCount = this.wikiInlineRules.length;
    var wikiToDomInlineRulesCount = this.wikiToDomInlineRules.length;
    var wikiRulesPattern = new RegExp(this.wikiRulesPattern.source, "g");

    var self = this;
    var fragment = contentDocument.createDocumentFragment();
    var holder = fragment;
    var lines = wikitext.split("\n");
    var codeText = null;
    var currentHeader = null;
    var quoteDepth = [];
    var listDepth = [];
    var decorationStatus;
    var decorationStack;
    var inCodeBlock, inParagraph, inDefList, inTable, inTableRow, continueTableRow;
    inCodeBlock = inParagraph = inDefList = inTable = inTableRow = continueTableRow = false;

    function handleCodeBlock(line) {
        if (/^ *\{\{\{ *$/.test(line)) {
            inCodeBlock++;
            if (inCodeBlock == 1) {
                closeParagraph();
                codeText = [];
            }
            else {
                codeText.push(line);
            }
        }
        else if (/^ *\}\}\} *$/.test(line)) {
            inCodeBlock--;
            if (inCodeBlock == 0) {
                var pre = contentDocument.createElement("pre");
                pre.className = "wiki";
                pre.appendChild(contentDocument.createTextNode(codeText.join(
                    pre.addEventListener && !window.opera ? "\n" : "\n\r")));
                holder.appendChild(pre);
                codeText = [];
            }
            else {
                codeText.push(line);
            }
        }
        else {
            codeText.push(line);
        }
    }

    function handleCitation(value) {
        var quote = /^(?: *>)+/.exec(value)[0];
        var depth = quote.replace(/ +/g, "").length;

        if (depth > quoteDepth.length) {
            closeToFragment("blockquote");
            while (depth > quoteDepth.length) {
                openQuote((new RegExp("^(?: *>){" + (quoteDepth.length + 1) + "}")).exec(quote)[0].length, true);
            }
        }
        else if (depth == quoteDepth.length) {
            // nothing to do
        }
        else {
            closeParagraph();
            while (depth < quoteDepth.length) {
                closeQuote();
            }
        }
    }

    function openQuote(length, citation) {
        var target = holder;
        if (target != fragment) {
            target = getSelfOrAncestor(target, "blockquote");
        }

        var element = contentDocument.createElement("blockquote");
        if (citation) {
            element.className = "citation";
        }
        (target || fragment).appendChild(element);
        holder = element;
        quoteDepth.push(length);
    }

    function closeQuote() {
        var target = getSelfOrAncestor(holder, "blockquote");
        holder = target.parentNode;
        quoteDepth.pop();
    }

    function handleHeader(line) {
        var match = /^\s*(=+)[ \t\r\f\v]+.*?(?:#([^ \t\r\f\v]+))?[ \t\r\f\v]*$/.exec(line);
        if (!match) {
            return null;
        }

        closeToFragment();
        var tag = "h" + match[1].length;
        var element = contentDocument.createElement(tag);
        if (match[2]) {
            element.id = match[2];
        }
        fragment.appendChild(element);
        holder = element;
        return tag;
    }

    function closeHeader() {
        if (currentHeader) {
            var target = getSelfOrAncestor(holder, currentHeader);
            holder = target.parentNode;
            currentHeader = null;
        }
    }

    function handleInline(name) {
        if (name == "bolditalic") {
            if (decorationStatus.italic) {
                handleInline("italic");
                handleInline("bold");
            }
            else {
                handleInline("bold");
                handleInline("italic");
            }
            return;
        }

        var d = contentDocument;
        if (decorationStatus[name]) {
            var tagNames = [];
            for (var index = decorationStack.length - 1; index >= 0; index--) {
                var tagName = holder.tagName;
                holder = holder.parentNode;
                if (decorationStack[index] == name) {
                    break;
                }
                tagNames.push(tagName);
            }
            decorationStack.splice(index, 1);
            decorationStatus[name] = false;
            while (tagNames.length > 0) {
                var element = d.createElement(tagNames.pop());
                holder.appendChild(element);
                holder = element;
            }
            return;
        }

        var tagName;
        switch (name) {
        case "bold":        tagName = "b";      break;
        case "italic":      tagName = "i";      break;
        case "underline":   tagName = "u";      break;
        case "strike":      tagName = "del";    break;
        case "subscript":   tagName = "sub";    break;
        case "superscript": tagName = "sup";    break;
        }

        if (holder == fragment) {
            openParagraph();
        }
        element = d.createElement(tagName);
        holder.appendChild(element);
        holder = element;
        decorationStatus[name] = true;
        decorationStack.push(name);
    }

    function handleInlineCode(value, length) {
        var d = contentDocument;
        var element = d.createElement("tt");
        value = value.slice(length, -length);
        if (value.length > 0) {
            element.appendChild(d.createTextNode(value));
            holder.appendChild(element);
        }
    }

    function createAnchor(link, label) {
        var anchor = self.createAnchor(link, label);
        holder.appendChild(anchor);
    }

    function handleTracLinks(value) {
        var match = handleTracLinks.pattern.exec(value);
        if (match) {
            var link = match[1];
            if (!/^(?:[\w.+-]+:|[\/.#].*)/.test(link)) {
                link = "wiki:" + link;
            }
            var text = (match[2] || match[1].replace(/^[\w.+-]+:/, "")).replace(/^(["'])(.*)\1$/g, "$2");
            createAnchor(link, text);
        }
        else {
            holder.appendChild(contentDocument.createTextNode(value));
        }
    }
    handleTracLinks.pattern = new RegExp("\\["
        + "((?:" + _linkScheme + ":)?(?:" + _quotedString + "|[^\\]\\s]+))"
        + "(?:\\s+(.*))?\\]");

    function handleTracWikiLink(value) {
        createAnchor(value, value);
    }

    function handleBracketLinks(value) {
        var d = contentDocument;
        var link = value.slice(1, -1);
        var anchor = self.createAnchor(link, link);
        var _holder = holder;
        _holder.appendChild(d.createTextNode("<"));
        _holder.appendChild(anchor);
        _holder.appendChild(d.createTextNode(">"));
    }

    function handleWikiPageName(name, label) {
        createAnchor("wiki:" + name, label || name);
    }

    function handleTracOtherLinks(value) {
        createAnchor(self.convertWikiSyntax(value), value);
    }

    function handleTracTicketLink(value) {
        if (!/^&/.test(value)) {
            handleTracOtherLinks(value);
            return true;
        }
        return false;
    }

    function handleWikiAnchor(text) {
        var match = /^\[=#([^ \t\r\f\v\]]+)(?:[ \t\r\f\v]+([^\]]*))?\]$/.exec(text);
        var d = contentDocument;
        var element = d.createElement("span");
        element.className = "wikianchor";
        element.id = match[1];
        if (match[2]) {
            element.appendChild(self.wikitextToOnelinerFragment(match[2], d, self.options));
        }
        holder.appendChild(element);
    }

    function handleList(value) {
        var match = /^(\s*)(?:([-*])|((?:([0-9]+)|([a-z])|([A-Z])|[ivxIVX]{1,5})))/.exec(value);
        var tag, className, depth, start;
        if (!match) {
            holder.appendChild(contentDocument.createTextNode(value));
            return;
        }

        depth = match[1].length;
        if (match[2]) {
            tag = "ul";
        }
        else if (match[3]) {
            tag = "ol";
            switch (match[3]) {
            case "0":
                className = "arabiczero";
                break;
            case "1":
                break;
            case "i":
                className = "lowerroman";
                break;
            case "I":
                className = "upperroman";
                break;
            default:
                if (match[4]) {
                    start = parseInt(match[4], 10);
                }
                else if (match[5]) {
                    className = "loweralpha";
                }
                else if (match[6]) {
                    className = "upperalpha";
                }
                break;
            }
        }

        var last = listDepth.length - 1;
        if (depth > (last >= 0 ? listDepth[last] : -1)) {
            closeToFragment("li");
            openList(tag, className, start, depth);
        }
        else {
            var container, list;
            if (listDepth.length > 1 && depth < listDepth[last]) {
                do {
                    if (depth >= listDepth[last]) {
                        break;
                    }
                    closeList();
                    last = listDepth.length - 1;
                } while (listDepth.length > 1);
                container = holder;
            }
            else {
                list = getSelfOrAncestor(holder, "li");
                self.appendBogusLineBreak(list);
                container = list.parentNode;
            }
            if (tag != container.tagName.toLowerCase()) {
                holder = container.parentNode;
                listDepth.pop();
                openList(tag, className, start, depth);
            }
            else {
                var tmp = contentDocument.createElement("li");
                container.appendChild(tmp);
                holder = tmp;
                listDepth[last] = depth;
            }
        }
    }

    function openList(tag, className, start, depth) {
        var d = contentDocument;
        var h = holder;

        var container = d.createElement(tag);
        if (className) {
            container.className = className;
        }
        if (start) {
            container.setAttribute("start", start);
        }
        var list = d.createElement("li");
        container.appendChild(list);

        var target;
        if (h == fragment) {
            target = fragment;
        }
        else {
            target = getSelfOrAncestor(h, "li");
            target = target ? target.parentNode : h;
        }
        target.appendChild(container);
        holder = list;
        listDepth.push(depth);
    }

    function closeList() {
        var h = holder;
        var target = getSelfOrAncestor(h, "li");
        if (target) {
            self.appendBogusLineBreak(target);
            holder = target.parentNode.parentNode;
        }
        else {
            holder = h.parentNode;
        }
        listDepth.pop();
    }

    function handleDefinition(value) {
        var d = contentDocument;
        var h = holder;
        var dl = null;
        if (inDefList) {
            dl = getSelfOrAncestor(h, "dl");
        }
        else {
            closeParagraph();
            dl = d.createElement("dl");
            fragment.appendChild(dl);
            inDefList = true;
        }

        var match = /^ +(.*?)\s*::/.exec(value);
        var dt = d.createElement("dt");
        var oneliner = self.wikitextToOnelinerFragment(match[1], d, self.options);
        dt.appendChild(oneliner);
        dl.appendChild(dt);

        var dd = d.createElement("dd");
        dl.appendChild(dd);
        holder = dd;
    }

    function closeDefList() {
        var element = getSelfOrAncestor(holder, "dl");
        if (element) {
            holder = element.parentNode;
        }
        inDefList = false;
    }

    function handleIndent(value) {
        var depth = value.length;
        var last = quoteDepth.length - 1;

        if (depth > (last >= 0 ? quoteDepth[last] : 0)) {
            closeParagraph();
            closeTable();
            openQuote(depth, false);
        }
        else {
            while (quoteDepth.length > 0) {
                if (depth >= quoteDepth[last]) {
                    break;
                }
                closeToFragment("blockquote");
                closeQuote();
                last = quoteDepth.length - 1;
            }
            quoteDepth[last] = depth;
        }
    }

    function openParagraph() {
        if (!inParagraph) {
            var element = contentDocument.createElement("p");
            holder.appendChild(element);
            holder = element;
            inParagraph = true;
        }
    }

    function closeParagraph() {
        if (inParagraph) {
            var target = holder;
            if (target != fragment) {
                target = getSelfOrAncestor(target, "p");
                self.appendBogusLineBreak(target);
            }
            holder = target.parentNode;
            inParagraph = false;
        }
    }

    function handleTableCell(action, colspan, header, align) {
        var d = contentDocument;
        var h, table, tbody;

        if (!inTable) {
            closeToFragment("blockquote");
            h = holder;
            table = d.createElement("table");
            table.className = "wiki";
            tbody = d.createElement("tbody");
            table.appendChild(tbody);
            h.appendChild(table);
            inTable = true;
            inTableRow = false;
        }
        else {
            h = holder;
            tbody = getSelfOrAncestor(h, "tbody");
        }

        if (inTableRow) {
            var cell = getSelfOrAncestor(h, "td");
            if (cell) {
                self.appendBogusLineBreak(cell);
            }
        }

        var row;
        switch (action) {
        case 1:
            row = d.createElement("tr");
            tbody.appendChild(row);
            inTableRow = true;
            break;
        case 0:
            row = getSelfOrAncestor(h, "tr");
            break;
        case -1:
            if (inTableRow) {
                var target = getSelfOrAncestor(h, "tr");
                holder = target.parentNode;
                inTableRow = false;
            }
            return;
        }

        var cell = d.createElement(header ? "th" : "td");
        if (colspan > 1) {
            cell.setAttribute("colSpan", colspan);
        }
        switch (align) {
            case -1:    align = "left";     break;
            case 0:     align = "center";   break;
            case 1:     align = "right";    break;
            default:    align = null;       break;
        }
        if (align !== null) {
            cell.setAttribute("align", align);
        }
        row.appendChild(cell);
        holder = cell;
        decorationStatus = {};
    }

    function closeTable() {
        if (inTable) {
            var target = getSelfOrAncestor(holder, "table");
            holder = target.parentNode;
            inTable = inTableRow = false;
        }
    }

    function closeToFragment(stopTag) {
        var element = holder;
        var _fragment = fragment;
        stopTag = stopTag ? stopTag.toLowerCase() : null;

        while (element != _fragment) {
            var tag = element.tagName.toLowerCase();
            if (tag == stopTag) {
                holder = element;
                return;
            }
            var method;
            switch (tag) {
            case "p":
                method = closeParagraph;
                break;
            case "li": case "ul": case "ol":
                method = closeList;
                break;
            case "dd":
                method = closeDefList;
                break;
            case "blockquote":
                method = closeQuote;
                break;
            case "td": case "tr": case "tbody": case "table":
                method = closeTable;
                break;
            default:
                break;
            }
            if (method) {
                method();
                element = holder;
            }
            else {
                element = element.parentNode;
            }
        }

        holder = _fragment;
    }

    function getMatchNumber(match) {
        var length = match.length;
        for (var i = 1; i < length; i++) {
            if (match[i]) {
                if (i <= wikiInlineRulesCount) {
                    return i;
                }
                if (i <= wikiToDomInlineRulesCount) {
                    return i - wikiInlineRulesCount + 1000;
                }
                return wikiToDomInlineRulesCount - i;
            }
        }
        return null;
    }

    for (var indexLines = 0; indexLines < lines.length; indexLines++) {
        var line = lines[indexLines].replace(/\r$/, "");
        if (inCodeBlock || /^ *\{\{\{ *$/.test(line)) {
            handleCodeBlock(line);
            continue;
        }
        if (/^----/.test(line)) {
            closeToFragment();
            fragment.appendChild(contentDocument.createElement("hr"));
            continue;
        }
        if (line.length == 0) {
            closeToFragment();
            continue;
        }
        line = line.replace(/\t/g, "        ");
        line = line.replace(/\u00a0/g, " ");

        wikiRulesPattern.lastIndex = 0;
        var prevIndex = wikiRulesPattern.lastIndex;
        decorationStatus = {};
        decorationStack = [];
        for ( ; ; ) {
            var match = wikiRulesPattern.exec(line);
            var matchNumber = null;
            var text = null;
            if (match) {
                matchNumber = getMatchNumber(match);
                if (prevIndex < match.index) {
                    text = line.substring(prevIndex, match.index);
                }
            }
            else {
                text = line.substring(prevIndex);
            }

            if ((prevIndex == 0 && text || match && match.index == 0 && matchNumber > 0)
                && (!inParagraph || quoteDepth.length > 0)
                && (!inDefList || !/^ /.test(line)))
            {
                closeToFragment();
            }
            if (text || match && matchNumber > 0) {
                if (inParagraph && (prevIndex == 0 || quoteDepth.length > 0)) {
                    if (escapeNewlines) {
                        if (quoteDepth.length == 0) {
                            holder.appendChild(contentDocument.createElement("br"));
                        }
                    }
                    else {
                        text = text ? (" " + text) : " ";
                    }
                }
                if (!inTable && quoteDepth.length > 0 || holder == fragment) {
                    if (!inParagraph) {
                        openParagraph();
                    }
                }
                if (text) {
                    holder.appendChild(contentDocument.createTextNode(text));
                }
            }
            if (!match) {
                break;
            }
            prevIndex = wikiRulesPattern.lastIndex;
            var matchText = match[0];

            if (!/^!/.test(matchText)) {    // start '!'
                switch (matchNumber) {
                case 1:     // bolditalic
                    handleInline("bolditalic");
                    continue;
                case 2:     // bold
                    handleInline("bold");
                    continue;
                case 3:     // italic
                    handleInline("italic");
                    continue;
                case 4:     // underline
                    handleInline("underline");
                    continue;
                case 5:     // strike
                    handleInline("strike");
                    continue;
                case 6:     // subscript
                    handleInline("subscript");
                    continue;
                case 7:     // superscript
                    handleInline("superscript");
                    continue;
                case 8:     // code block
                    handleInlineCode(matchText, 3);
                    continue;
                case 9:     // inline
                    handleInlineCode(matchText, 1);
                    continue;
                case 10:    // ticket
                    if (handleTracTicketLink(matchText)) {
                        continue;
                    }
                    break;
                case 11:    // report
                case 12:    // changeset
                case 13:    // log
                    handleTracOtherLinks(matchText);
                    continue;
                case 14:    // wiki:TracLinks
                    handleTracWikiLink(matchText);
                    continue;
                case 15:    // [wiki:TracLinks label]
                    handleTracLinks(matchText);
                    continue;
                case 16:    // [[macro]]
                    break;
                case 17:    // WikiPageName
                    handleWikiPageName(matchText);
                    continue;
                case 18:    // ["internal free link"]
                    handleWikiPageName(matchText.slice(1, -1), matchText.slice(2, -2));
                    continue;
                case 19:    // <wiki:Trac bracket links>
                    handleBracketLinks(matchText);
                    continue;
                case 20:    // [=#anchor label]
                    handleWikiAnchor(matchText);
                    continue;
                case 1001:  // escaping double escape
                    break;
                case -1:    // citation
                    if (escapeNewlines && inParagraph) {
                        holder.appendChild(contentDocument.createElement("br"));
                    }
                    handleCitation(matchText);
                    if (escapeNewlines) {
                        openParagraph();
                    }
                    continue;
                case -2:    // header
                    currentHeader = handleHeader(matchText);
                    if (currentHeader) {
                        line = line.replace(/(?:[ \t\r\f\v]+#[^ \t\r\f\v]+)?[ \t\r\f\v]*$/, "");
                        var m = /^\s*(=+)[ \t\r\f\v]+/.exec(line);
                        if (line.slice(-m[1].length) == m[1]) {
                            line = line.slice(0, -m[1].length).replace(/[ \t\r\f\v]+$/, "");
                        }
                        wikiRulesPattern.lastIndex = prevIndex = m[0].length;
                        continue;
                    }
                    break;
                case -3:    // list
                    handleList(matchText)
                    continue;
                case -4:    // definition
                    handleDefinition(matchText);
                    continue;
                case -5:    // leading space
                    if (listDepth.length == 0 && !inDefList) {
                        handleIndent(matchText);
                        continue;
                    }
                    if (!this.isInlineNode(holder.lastChild)) {
                        continue;
                    }
                    matchText = matchText.replace(/^\s+/, " ");
                    break;
                case -6:    // closing table row
                    if (inTable) {
                        if (matchText.slice(-1) != "\\") {
                            handleTableCell(-1);
                        }
                        else {
                            continueTableRow = true;
                        }
                        continue;
                    }
                    break;
                case -7:    // cell
                    if (quoteDepth.length > 0 && match.index == 0) {
                        closeToFragment();
                    }
                    var align = null;
                    for ( ; ; ) {       // lookahead next double pipes
                        var m = wikiRulesPattern.exec(line);
                        switch (m ? getMatchNumber(m) : 0) {
                        case 0: case -6: case -7:
                            var end = m ? m.index : line.length;
                            if (prevIndex < end) {
                                var tmp = line.substring(prevIndex, end);
                                var m = /^([ \t\r\n\f\v]*)(.*?)([ \t\r\n\f\v]*)$/.exec(tmp);
                                if (m) {
                                    if (m[1].length == tmp.length) {
                                        align = null;
                                    }
                                    else if ((m[1].length == 0) === (m[3].length == 0)) {
                                        align = m[1].length >= 2 && m[3].length >= 2 ? 0 : null;
                                    }
                                    else {
                                        align = m[1].length == 0 ? -1 : 1;
                                    }
                                    tmp = m[2];
                                }
                                line = line.substring(0, prevIndex) + tmp + line.substring(end);
                            }
                            break;
                        default:
                            continue;
                        }
                        break;
                    }
                    wikiRulesPattern.lastIndex = prevIndex;
                    handleTableCell(inTableRow ? 0 : 1,
                        matchText.replace(/^=|=$/g, '').length / 2, matchText.slice(-1) == "=", align);
                    continue;
                }
            }

            if (matchText) {
                if (listDepth.length == 0 && !currentHeader && !inDefList && !inTable) {
                    openParagraph();
                }
                var tmp;
                if (matchNumber == 16) {
                    tmp = /^!?\[\[br\]\]$/i.test(matchText)
                        ? (matchText.charCodeAt(0) == 0x21
                            ? contentDocument.createTextNode(matchText.substring(1))
                            : contentDocument.createElement("br"))
                        : contentDocument.createTextNode(matchText);
                }
                else {
                    tmp = contentDocument.createTextNode(/^!/.test(matchText) ? matchText.substring(1) : matchText);
                }
                holder.appendChild(tmp);
            }
        }
        if (currentHeader) {
            closeHeader();
        }
        if (inTable) {
            if (continueTableRow) {
                continueTableRow = false;
            }
            else {
                handleTableCell(-1);
            }
        }
    }
    closeToFragment();

    return fragment;
};

prototype.wikitextToOnelinerFragment = function(wikitext, contentDocument, options) {
    var source = this.wikitextToFragment(wikitext, contentDocument, options);
    var fragment = contentDocument.createDocumentFragment();
    this.collectChildNodes(fragment, source.firstChild);
    return fragment;
};

prototype.wikiOpenTokens = {
    "h1": "= ", "h2": "== ", "h3": "=== ", "h4": "==== ", "h5": "===== ", "h6": "====== ",
    "b": "'''", "strong": "'''",
    "i": "''", "em": "''",
    "u": "__",
    "del": "~~", "strike": "~~",
    "sub": ",,",
    "sup": "^",
    "hr": "----\n",
    "dl": true,
    "dt": " ",
    "dd": " ",
    "table": true,
    "tbody": true };

prototype.wikiCloseTokens = {
    "#text": true,
    "a": true,
    "tt": true,
    "h1": " =", "h2": " ==", "h3": " ===", "h4": " ====", "h5": " =====", "h6": " ======",
    "b": "'''", "strong": "'''",
    "i": "''", "em": "''",
    "u": "__",
    "del": "~~", "strike": "~~",
    "sub": ",,",
    "sup": "^",
    "br": true,
    "hr": true,
    "dl": "\n",
    "dt": "::",
    "dd": "\n",
    "tbody": true,
    "tr": "||\n",
    "td": true, "th": true };

prototype.wikiBlockTags = {
    "h1": true, "h2": true, "h3": true, "h4": true, "h5": true, "h6": true,
    "table": true, "dl": true, "hr": true };

prototype.wikiInlineTags = {
    "a": true, "tt": true, "b": true, "strong": true, "i": true, "em": true,
    "u": true, "del": true, "strike": true, "sub": true, "sup": true,
    "br": true, "span": true };

prototype.domToWikitext = function(root, options) {
    options = options || {};
    var formatCodeBlock = !!options.formatCodeBlock;
    var escapeNewlines = !!options.escapeNewlines;

    var self = this;
    var getTextContent = TracWysiwyg.getTextContent;
    var getSelfOrAncestor = TracWysiwyg.getSelfOrAncestor;
    var wikiOpenTokens = this.wikiOpenTokens;
    var wikiCloseTokens = this.wikiCloseTokens;
    var wikiInlineTags = this.wikiInlineTags;
    var wikiBlockTags = this.wikiBlockTags;
    var xmlNamePattern = this.xmlNamePattern;
    var domToWikiInlinePattern = this.domToWikiInlinePattern;
    var wikiSyntaxPattern = this.wikiSyntaxPattern;
    var tracLinkPattern = new RegExp("^" + this._tracLink + "$");
    var wikiPageNamePattern = new RegExp("^" + this._wikiPageName + "$");
    var decorationTokenPattern = /^(?:'''|''|__|\^|,,)$/;

    var texts = [];
    var stack = [];
    var last = root;
    var listDepth = 0;
    var quoteDepth = 0;
    var quoteCitation = false;
    var inCodeBlock = false;
    var skipNode = null;
    var openBracket = false;

    function escapeText(s) {
        var match = /^!?\[\[(.+)\]\]$/.exec(s);
        if (match) {
            return match[1].toLowerCase() != "br" ? s : "!" + s;
        }
        if (/^&#\d+/.test(s)) {
            return s;
        }
        return "!" + s;
    }

    function isTailEscape() {
        var t = texts;
        var length = t.length;
        return length > 0 ? /!$/.test(t[length - 1]) : false;
    }

    function tokenFromSpan(node) {
        if (node.className == "underline") {
            return wikiOpenTokens["u"];
        }
        var style = node.style;
        if (style.fontWeight == "bold") {
            return wikiOpenTokens["b"];
        }
        if (style.fontStyle == "italic") {
            return wikiOpenTokens["i"];
        }
        switch (style.textDecoration) {
        case "underline":
            return wikiOpenTokens["u"];
        case "line-through":
            return wikiOpenTokens["del"];
        }
        switch (style.verticalAlign) {
        case "sub":
            return wikiOpenTokens["sub"];
        case "sup":
            return wikiOpenTokens["sup"];
        }
        return undefined;
    }

    function nodeDecorations(node) {
        var _wikiOpenTokens = wikiOpenTokens;
        var _decorationTokenPattern = decorationTokenPattern;
        var hash = {};

        for ( ; ; ) {
            var childNodes = node.childNodes;
            if (!childNodes || childNodes.length != 1) {
                break;
            }
            var child = childNodes[0];
            if (child.nodeType != 1) {
                break;
            }
            var token = _wikiOpenTokens[child.tagName.toLowerCase()];
            if (_decorationTokenPattern.test(token)) {
                hash[token] = true;
            }
            node = child;
        }

        return hash;
    }

    function pushTextWithDecorations(text, node, traclink) {
        var _texts = texts;
        var _decorationTokenPattern = decorationTokenPattern;
        var decorationsHash = nodeDecorations(node);
        var decorations = [];
        var cancelDecorations = [];

        while (_texts.length > 0) {
            var token = _texts[_texts.length - 1];
            if (_decorationTokenPattern.test(token)) {
                if (decorationsHash[token]) {
                    delete decorationsHash[token];
                    cancelDecorations.push(_texts.pop());
                    continue;
                }
                if ((token == "'''" || token == "''") && _texts.length > 1) {
                    var moreToken = _texts[_texts.length - 2];
                    if (_decorationTokenPattern.test(moreToken)
                        && token + moreToken == "'''''"
                        && decorationsHash[moreToken])
                    {
                        delete decorationsHash[moreToken];
                        cancelDecorations.push(moreToken);
                        _texts[_texts.length - 2] = _texts[_texts.length - 1];
                        _texts.pop();
                    }
                }
            }
            break;
        }

        for (var token in decorationsHash) {
            decorations.push(token);
        }
        decorations.sort();

        if (decorations.length > 0) {
            _texts.push.apply(_texts, decorations);
        }
        if (traclink) {
            if (_texts.length > 0 && /[\w.+-]$/.test(_texts[_texts.length - 1])) {
                _texts.push(traclink);
            }
            else {
                text = new String(text);
                text["tracwysiwyg-traclink"] = traclink;
                _texts.push(text);
            }
        }
        else {
            _texts.push(text);
        }
        if (decorations.length > 0) {
            decorations.reverse();
            _texts.push.apply(_texts, decorations);
        }
        if (cancelDecorations.length > 0) {
            cancelDecorations.reverse();
            _texts.push.apply(_texts, cancelDecorations);
        }
    }

    function pushToken(token) {
        var _texts = texts;
        var _decorationTokenPattern = decorationTokenPattern;
        var length = _texts.length;
        if (length == 0 || !_decorationTokenPattern.test(token)) {
            _texts.push(token);
            return;
        }
        var last = _texts[length - 1];
        if (!_decorationTokenPattern.test(last)) {
            _texts.push(token);
            return;
        }
        if (last == token) {
            _texts.pop();
            return;
        }
        if (length < 2 || last + token != "'''''") {
            _texts.push(token);
            return;
        }
        if (_texts[length - 2] == token) {
            _texts[length - 2] = _texts[length - 1];
            _texts.pop();
        }
        else {
            _texts.push(token);
        }
    }

    function tracLinkText(link, label) {
        if (!/\]/.test(label) && !/^[\"\']/.test(label)) {
            return "[" + link + " " + label + "]";
        }
        if (!/\"/.test(label)) {
            return "[" + link + ' "' + label + '"]';
        }
        if (!/\'/.test(label)) {
            return "[" + link + " '" + label + "']";
        }
        return "[" + link + ' "' + label.replace(/"+/g, "") + '"]';
    }

    function pushAnchor(node, bracket) {
        var link = node.getAttribute("data-tracwysiwyg-link");
        var autolink = node.getAttribute("data-tracwysiwyg-autolink");
        var attrs;
        if (link === null) {
            attrs = TracWysiwyg.unserializeFromHref(node.href);
            link = attrs["data-tracwysiwyg-link"];
            autolink = attrs["data-tracwysiwyg-autolink"];
        }
        link = (link || node.href).replace(/^\s+|\s+$/g, "");
        var label = getTextContent(node).replace(/^\s+|\s+$/g, "");
        if (!label) {
            return;
        }
        var text = null;
        var traclink = null;
        if (autolink == "true") {
            if (wikiPageNamePattern.test(label)) {
                text = label;
                link = "wiki:" + label;
                traclink = "[wiki:" + label + "]";
            }
            else if (wikiSyntaxPattern.test(label)) {
                text = label;
                link = self.convertWikiSyntax(label);
            }
            else if (tracLinkPattern.test(label)) {
                text = link = label;
            }
        }
        else {
            if (link == label) {
                if (bracket) {
                    text = label;
                }
                else if (tracLinkPattern.test(label)) {
                    text = label;
                }
            }
        }
        if (!text) {
            var match = /^([\w.+-]+):(@?(.*))$/.exec(link);
            if (match) {
                if (label == match[2]) {
                    if (match[1] == "wiki" && wikiPageNamePattern.test(match[2])) {
                        text = match[2];
                        traclink = "[wiki:" + text + "]";
                    }
                    else {
                        text = "[" + link + "]";
                    }
                }
                else {
                    var usingLabel = false;
                    switch (match[1]) {
                    case "changeset":
                        usingLabel = label == "[" + match[2] + "]" || /^\d+$/.test(match[2]) && label == "r" + match[2];
                        break;
                    case "log":
                        usingLabel = label == "[" + match[3] + "]" || label == "r" + match[3];
                        break;
                    case "report":
                        usingLabel = label == "{" + match[2] + "}";
                        break;
                    case "ticket":
                        usingLabel = label == "#" + match[2];
                        break;
                    }
                    if (usingLabel) {
                        text = label;
                    }
                }
            }
        }
        if (isTailEscape()) {
            texts.push(" ");
        }
        if (text === null) {
            text = tracLinkText(link, label);
        }
        if (!traclink && /^[\w.+-]/.test(text)) {
            traclink = tracLinkText(link, label);
        }
        pushTextWithDecorations(text, node, traclink);
    }

    function string(source, times) {
        var value = (1 << times) - 1;
        if (value <= 0) {
            return "";
        }
        else {
            return value.toString(2).replace(/1/g, source);
        }
    }

    function open(name, node) {
        if (skipNode !== null) {
            return;
        }
        var _texts = texts;
        var token = wikiOpenTokens[name];
        if (token !== undefined) {
            if (name in wikiBlockTags && self.isInlineNode(node.previousSibling)) {
                _texts.push("\n");
            }
            if (token !== true) {
                if (name in wikiInlineTags && isTailEscape()) {
                    _texts.push(" ");
                }
                pushToken(token);
            }
            openBracket = false;
        }
        else {
            switch (name) {
            case "#text":
                var value = node.nodeValue;
                if (value) {
                    if (!inCodeBlock) {
                        if (value && !self.isInlineNode(node.previousSibling || node.parentNode)) {
                            value = value.replace(/^[ \t\r\n\f\v]+/g, "");
                        }
                        if (value && !self.isInlineNode(node.nextSibling || node.parentNode)) {
                            value = value.replace(/[ \t\r\n\f\v]+$/g, "");
                        }
                        value = value.replace(/\r?\n/g, " ");
                        if (!formatCodeBlock) {
                            value = value.replace(domToWikiInlinePattern, escapeText);
                        }
                        openBracket = /<$/.test(value);
                    }
                    if (value) {
                        var length = _texts.length;
                        var prev = length > 0 ? _texts[length - 1] : null;
                        if (prev && prev["tracwysiwyg-traclink"] && tracLinkPattern.test(prev + value.substring(0, 1))) {
                            _texts[length - 1] = prev["tracwysiwyg-traclink"];
                        }
                        _texts.push(value);
                    }
                }
                break;
            case "p":
                if (quoteDepth > 0) {
                    _texts.push(string(quoteCitation ? "> " : "  ", quoteDepth));
                }
                else if (!/[^ \t\r\n\f\v]/.test(getTextContent(node))) {
                    skipNode = node;
                }
                break;
            case "a":
                skipNode = node;
                var bracket = false;
                if (openBracket) {
                    var nextSibling = node.nextSibling;
                    bracket = nextSibling && nextSibling.nodeType == 3 && /^>/.test(nextSibling.nodeValue);
                    openBracket = false;
                }
                pushAnchor(node, bracket);
                break;
            case "li":
                _texts.push(" " + string("  ", listDepth - 1));
                var container = node.parentNode;
                if ((container.tagName || "").toLowerCase() == "ol") {
                    var start = container.getAttribute("start") || "";
                    if (start != "1" && /^(?:[0-9]+|[a-zA-Z]|[ivxIVX]{1,5})$/.test(start)) {
                        _texts.push(start, ". ");
                    }
                    else {
                        switch (container.className) {
                        case "arabiczero":  _texts.push("0. "); break;
                        case "lowerroman":  _texts.push("i. "); break;
                        case "upperroman":  _texts.push("I. "); break;
                        case "loweralpha":  _texts.push("a. "); break;
                        case "upperalpha":  _texts.push("A. "); break;
                        default:            _texts.push("1. "); break;
                        }
                    }
                }
                else {
                    _texts.push("* ");
                }
                break;
            case "ul": case "ol":
                if (listDepth == 0) {
                    if (self.isInlineNode(node.previousSibling)) {
                        _texts.push("\n");
                    }
                }
                else if (listDepth > 0) {
                    if (node.parentNode.tagName.toLowerCase() == "li") {
                        _texts.push("\n");
                    }
                }
                listDepth++;
                break;
            case "br":
                if (!self.isBogusLineBreak(node)) {
                    var value = null;
                    if (inCodeBlock) {
                        value = "\n";
                    }
                    else if (formatCodeBlock) {
                        switch (((node.parentNode || {}).tagName || "").toLowerCase()) {
                        case "li":
                            value = "\n " + string("  ", listDepth);
                            break;
                        case "p": case "blockquote":
                            value = "\n";
                            if (quoteDepth > 0) {
                                value += string(quoteCitation ? "> " : "  ", quoteDepth);
                            }
                            break;
                        case "dd":
                            value = "\n    ";
                            break;
                        case "dt":
                        case "h1": case "h2": case "h3": case "h4": case "h5": case "h6":
                            value = " ";
                            break;
                        default:
                            value = "\n";
                            break;
                        }
                    }
                    else {
                        if (escapeNewlines && getSelfOrAncestor(node, /^(?:p|blockquote)$/)) {
                            value = quoteDepth > 0
                                ? "\n" + string(quoteCitation ? "> " : "  ", quoteDepth)
                                : "\n";
                        }
                        if (!value) {
                            value = "[[BR]]";
                            var length = _texts.length;
                            if (length > 0) {
                                var lastText = _texts[length - 1];
                                var tmp = lastText + "[[BR]]";
                                var _pattern = domToWikiInlinePattern;
                                _pattern.lastIndex = 0;
                                var lastMatch, match;
                                while (match = _pattern.exec(tmp)) {
                                    lastMatch = match;
                                }
                                if (lastMatch && lastMatch.index < lastText.length
                                    && lastMatch.index + lastMatch[0].length > lastText.length)
                                {
                                    value = " [[BR]]";
                                }
                            }
                        }
                    }
                    _texts.push(value);
                }
                break;
            case "pre":
                _texts.push(
                    /^(?:li|dd)$/i.test(node.parentNode.tagName) || self.isInlineNode(node.previousSibling)
                    ? "\n{{{\n" : "{{{\n");
                inCodeBlock = true;
                break;
            case "blockquote":
                if (self.isInlineNode(node.previousSibling)) {
                    _texts.push("\n");
                }
                quoteDepth++;
                if (quoteDepth == 1) {
                    quoteCitation = (node.className == "citation");
                }
                break;
            case "th":
                var header = true;
            case "td":
                skipNode = node;
                var colspan = node.getAttribute("colSpan");
                colspan = colspan ? parseInt(colspan, 10) : 0;
                _texts.push(colspan > 1 ? string("||", colspan) : "||");
                if (header) {
                    _texts.push("=");
                }
                var align = node.style.textAlign;
                if (!align) {
                    align = (node.getAttribute("align") || "").toLowerCase();
                }
                var text = self.domToWikitext(node, self.options).replace(/ *\n/g, "[[BR]]").replace(/^ +| +$/g, "");
                if (text) {
                    switch (align) {
                        case "left":    _texts.push(text, " ");         break;
                        case "center":  _texts.push("  ", text, "  ");  break;
                        case "right":   _texts.push(" ", text);         break;
                        default:        _texts.push(" ", text, " ");    break;
                    }
                }
                else {
                    _texts.push(" ");
                }
                if (header) {
                    _texts.push("=");
                }
                break;
            case "tr":
                if (quoteDepth > 0) {
                    _texts.push(string(quoteCitation ? ">" : "  ", quoteDepth));
                }
                break;
            case "tt":
                skipNode = node;
                var value = getTextContent(node);
                var text;
                if (value) {
                    if (isTailEscape()) {
                        _texts.push(" ");
                    }
                    if (!/`/.test(value)) {
                        text = "`" + value + "`";
                    }
                    else if (!/\{\{\{|\}\}\}/.test(value)) {
                        text = "{{{" + value + "}}}";
                    }
                    else {
                        text = value.replace(/[^`]+|`+/g, function(m) {
                            return m.charCodeAt(0) != 0x60 ? "`" + m + "`" : "{{{" + m + "}}}";
                        });
                    }
                    pushTextWithDecorations(text, node);
                }
                break;
            case "span":
                if (node.className == "wikianchor" && xmlNamePattern.test(node.id || "")) {
                    skipNode = node;
                    var text = self.domToWikitext(node, self.options).replace(/^ +| +$|\]/g, "");
                    _texts.push("[=#", node.id, text ? " " + text + "]" : "]");
                }
                else {
                    var token = tokenFromSpan(node);
                    if (token !== undefined) {
                        if (name in wikiInlineTags && isTailEscape()) {
                            _texts.push(" ");
                        }
                        pushToken(token);
                    }
                }
                break;
            case "script":
            case "style":
                skipNode = node;
                break;
            }
            if (name != "#text") {
                openBracket = false;
            }
        }
    }

    function close(name, node) {
        if (skipNode !== null) {
            if (skipNode == node) {
                skipNode = null;
            }
            return;
        }
        var _texts = texts;
        var token = wikiCloseTokens[name];
        if (token === true) {
            // nothing to do
        }
        else if (token !== undefined) {
            if (name in wikiInlineTags && isTailEscape()) {
                _texts.push(" ");
            }
            pushToken(token);
        }
        else {
            switch (name) {
            case "p":
                _texts.push(quoteDepth == 0 ? "\n\n" : "\n");
                break;
            case "li":
                if (node.getElementsByTagName("li").length == 0) {
                    _texts.push("\n");
                }
                break;
            case "ul": case "ol":
                listDepth--;
                if (listDepth == 0) {
                    _texts.push("\n");
                }
                break;
            case "pre":
                var text;
                var parentNode = node.parentNode;
                if (parentNode && /^(?:li|dd)$/i.test(parentNode.tagName)) {
                    var nextSibling = node.nextSibling;
                    if (!nextSibling) {
                        text = "\n}}}";
                    }
                    else if (nextSibling.nodeType != 1) {
                        text = "\n}}}\n";
                    }
                    else if (nextSibling.tagName.toLowerCase() == "pre") {
                        text = "\n}}}";
                    }
                    else {
                        text = "\n}}}\n";
                    }
                    if (text.slice(-1) == "\n") {
                        text += listDepth > 0 ? " " + string("  ", listDepth) : "    ";
                    }
                }
                else {
                    text = "\n}}}\n";
                }
                _texts.push(text);
                inCodeBlock = false;
                break;
            case "blockquote":
                quoteDepth--;
                if (quoteDepth == 0) {
                    _texts.push("\n");
                }
                break;
            case "span":
                var token = tokenFromSpan(node);
                if (token !== undefined) {
                    if (name in wikiInlineTags && isTailEscape()) {
                        _texts.push(" ");
                    }
                    _texts.push(token);
                }
                break;
            case "table":
                if (quoteDepth == 0) {
                    _texts.push("\n");
                }
                break;
            }
        }
        if (/^h[1-6]$/.test(name)) {
            if (xmlNamePattern.test(node.id || "")) {
                _texts.push(" #", node.id);
            }
            _texts.push("\n");
        }
    }

    function iterator(node) {
        var name = null;
        switch (node && node.nodeType) {
        case 1: // element
            name = node.tagName.toLowerCase();
            break;
        case 3: // text
            name = "#text";
            break;
        }

        if (node && last == node.parentNode) {  // down
            // nothing to do
        }
        else if (node && last == node.previousSibling) {    // forward
            close(stack.pop(), last);
        }
        else {  // up, forward
            var tmp = last;
            var nodeParent = node ? node.parentNode : root;
            for ( ; ; ) {
                var parent = tmp.parentNode;
                if (parent == node) {
                    break;
                }
                close(stack.pop(), tmp);
                if (parent == nodeParent || !parent) {
                    if (!node) {
                        return;
                    }
                    break;
                }
                tmp = parent;
            }
        }
        open(name, node);
        stack.push(name);
        last = node;
    }

    this.treeWalk(root, iterator);
    return texts.join("").replace(/^(?: *\n)+|(?: *\n)+$/g, "");
};

prototype._msieInsertHTML = function(html) {
    this.contentWindow.focus();
    var selection = this.contentDocument.selection;
    var range = selection.createRange();
    range.pasteHTML(html.replace(/\t/g, "&#9;"));
    range.collapse(false);
    range.select();
};

prototype._fragmentInsertHTML = function(html) {
    var range = this.getNativeSelectionRange();
    if (range) {
        var d = this.contentDocument;
        var tmp = d.createRange();
        tmp.setStart(d.body, 0);
        tmp.setEnd(d.body, 0);
        var fragment = tmp.createContextualFragment(html);
        range.deleteContents();
        range.insertNode(fragment);
        range.detach();
        tmp.detach();
    }
};

prototype.insertLineBreak = function() {
    this.insertHTML('<br>');
};

prototype.insertLineBreakOnShiftEnter = null;

if (window.getSelection) {
    prototype.appendBogusLineBreak = function(element) {
        var wikiInlineTags = this.wikiInlineTags;
        var last = element.lastChild;
        for ( ; ; ) {
            if (!last) {
                break;
            }
            if (last.nodeType != 1) {
                return;
            }
            var name = last.tagName.toLowerCase();
            if (name == "br") {
                break;
            }
            if (!(name in wikiInlineTags)) {
                return;
            }
            last = last.lastChild || last.previousSibling;
        }
        var br = this.contentDocument.createElement("br");
        element.appendChild(br);
    };
    prototype.isBogusLineBreak = prototype.isLastChildInBlockNode;
    prototype.insertParagraphOnEnter = function(event) {
        var range = this.getSelectionRange();
        var node = range.endContainer;
        var header = null;
        if (node && node.nodeType == 3 && range.endOffset == node.nodeValue.length) {
            var next = node.nextSibling;
            if (next === null || next.nodeType === 1 && next.tagName === 'BR') {
                while (node) {
                    if (node.nodeType === 1 && /^h[1-6]$/i.exec(node.tagName)) {
                        header = node;
                        break;
                    }
                    node = node.parentNode;
                }
                if (header) {
                    var parent = header.parentNode;
                    var childNodes = parent.childNodes;
                    var length = childNodes.length;
                    for (var offset = 0; offset < length; offset++) {
                        if (childNodes[offset] == header) {
                            offset++;
                            break;
                        }
                    }
                    this.selectRange(parent, offset, parent, offset);
                    this.insertHTML('<p><br /></p>');
                    TracWysiwyg.stopEvent(event);
                }
            }
        }
    };
    prototype.tableHTML = function(id, row, col) {
        var html = this._tableHTML(row, col);
        return html.replace(/<td><\/td>/g, '<td><br></td>').replace(/<td>/, '<td id="' + id + '">');
    };
    prototype.insertTableCell = function(row, index) {
        var cell = row.insertCell(index);
        this.appendBogusLineBreak(cell);
        return cell;
    };
    prototype.getFocusNode = function() {
        return this.contentWindow.getSelection().focusNode;
    };
    if (window.getSelection().setBaseAndExtent) {  // Safari 2+
        prototype.insertLineBreak = function() {
            this.execCommand("insertlinebreak");
        };
        prototype.insertLineBreakOnShiftEnter = function(event) {
            this.insertLineBreak();
            TracWysiwyg.stopEvent(event);
        };
    }
    if (window.getSelection().removeAllRanges) {
        prototype.selectNode = function(node) {
            var selection = this.contentWindow.getSelection();
            selection.removeAllRanges();
            var range = this.contentDocument.createRange();
            range.selectNode(node);
            selection.addRange(range);
        };
        prototype.selectNodeContents = function(node) {
            var selection = this.contentWindow.getSelection();
            selection.removeAllRanges();
            var range = this.contentDocument.createRange();
            range.selectNodeContents(node);
            selection.addRange(range);
        };
        prototype.selectRange = function(start, startOffset, end, endOffset) {
            var selection = this.contentWindow.getSelection();
            selection.removeAllRanges();
            var range = this.contentDocument.createRange();
            range.setStart(start, startOffset);
            range.setEnd(end, endOffset);
            selection.addRange(range);
        };
        prototype.getNativeSelectionRange = function() {
            var selection = this.contentWindow.getSelection();
            return selection.rangeCount > 0 ? selection.getRangeAt(0) : null;
        };
        prototype.expandSelectionToElement = function(arg) {
            if (arg.start || arg.end) {
                var selection = this.contentWindow.getSelection();
                var range = this.getNativeSelectionRange() || this.contentDocument.createRange();
                selection.removeAllRanges();
                if (arg.start) {
                    range.setStartBefore(arg.start);
                }
                if (arg.end) {
                    range.setEndAfter(arg.end);
                }
                selection.addRange(range);
            }
        };
        if (window.TextRange === undefined || !TextRange.prototype.pasteHTML) {
            prototype.insertHTML = function(html) {
                this.execCommand("inserthtml", html);
            };
        }
        else if (document.selection === undefined) {  // Internet Explorer 11
            prototype.insertHTML = prototype._fragmentInsertHTML;
        }
        else {  // Internet Explorer 9 or 10
            prototype.insertHTML = prototype._msieInsertHTML;
        }
    }
    else {      // Safari 2
        prototype.selectNode = function(node) {
            var selection = this.contentWindow.getSelection();
            var range = this.contentDocument.createRange();
            range.selectNode(node);
            selection.setBaseAndExtent(range.startContainer, range.startOffset, range.endContainer, range.endOffset);
            range.detach();
        };
        prototype.selectNodeContents = function(node) {
            this.selectRange(node, 0, node, node.childNodes.length);
        };
        prototype.selectRange = function(start, startOffset, end, endOffset) {
            var selection = this.contentWindow.getSelection();
            selection.setBaseAndExtent(start, startOffset, end, endOffset);
        };
        prototype.getNativeSelectionRange = function() {
            var selection = this.contentWindow.getSelection();
            if (selection.anchorNode) {
                var range = this.contentDocument.createRange();
                range.setStart(selection.baseNode, selection.baseOffset);
                range.setEnd(selection.extentNode, selection.extentOffset);
                if (range.collapsed && !selection.isCollapsed) {
                    range.setStart(selection.extentNode, selection.extentOffset);
                    range.setEnd(selection.baseNode, selection.baseOffset);
                }
                return range;
            }
            return null;
        };
        prototype.expandSelectionToElement = function(arg) {
            if (arg.start || arg.end) {
                var selection = this.contentWindow.getSelection();
                var range = this.getNativeSelectionRange();
                if (arg.start) {
                    range.setStartBefore(arg.start);
                }
                if (arg.end) {
                    range.setEndAfter(arg.end);
                }
                selection.setBaseAndExtent(range.startContainer, range.startOffset, range.endContainer, range.endOffset);
                range.detach();
            }
        };
        prototype.insertHTML = prototype._fragmentInsertHTML;
    }
    prototype.getSelectionRange = prototype.getNativeSelectionRange;
    prototype.getSelectionText = function() {
        var range = this.getNativeSelectionRange();
        return range ? range.toString() : null;
    };
    prototype.getSelectionHTML = function() {
        var fragment = this.getSelectionFragment();
        var anonymous = this.contentDocument.createElement("div");
        anonymous.appendChild(fragment);
        return anonymous.innerHTML;
    };
    prototype.getSelectionFragment = function() {
        var range = this.getNativeSelectionRange();
        return range ? range.cloneContents() : this.contentDocument.createDocumentFragment();
    };
    prototype.getSelectionPosition = function() {
        var range = this.getNativeSelectionRange();
        var position = { start: null, end: null };
        if (range) {
            position.start = range.startContainer;
            position.end = range.endContainer;
        }
        return position;
    };
    prototype.selectionContainsTagName = function(name) {
        var selection = this.contentWindow.getSelection();
        var range = this.getNativeSelectionRange();
        if (!range) {
            return false;
        }
        var ancestor = range.commonAncestorContainer;
        if (!ancestor) {
            return false;
        }
        if (TracWysiwyg.getSelfOrAncestor(ancestor, name)) {
            return true;
        }
        if (ancestor.nodeType != 1) {
            return false;
        }
        var START_TO_END = Range.START_TO_END;
        var END_TO_START = Range.END_TO_START;
        var d = this.contentDocument;
        var elements = ancestor.getElementsByTagName(name);
        var length = elements.length;
        for (var i = 0; i < length; i++) {
            var source = d.createRange();
            source.selectNode(elements[i]);
            if (range.compareBoundaryPoints(START_TO_END, source) === -1 &&
                range.compareBoundaryPoints(END_TO_START, source) === 1)
            {
                return true;
            }
        }
        return false;
    };
}
else if (document.selection) {
    prototype.appendBogusLineBreak = function(element) { };
    prototype.isBogusLineBreak = function(node) { return false };
    prototype.insertParagraphOnEnter = null;
    prototype.tableHTML = function(id, row, col) {
        var html = this._tableHTML(row, col);
        return html.replace(/<td>/, '<td id="' + id + '">');
    };
    prototype.insertTableCell = function(row, index) {
        return row.insertCell(index);
    };
    prototype.getFocusNode = function() {
        this.contentWindow.focus();
        var d = this.contentDocument;
        var range = d.selection.createRange();
        var node = range.item ? range.item(0) : range.parentElement();
        return node.ownerDocument == d ? node : null;
    };
    prototype.selectNode = function(node) {
        var d = this.contentDocument;
        var body = d.body;
        var range;
        d.selection.empty();
        try {
            range = body.createControlRange();
            range.addElement(node);
        }
        catch (e) {
            range = body.createTextRange();
            range.moveToElementText(node);
        }
        range.select();
    };
    prototype.selectNodeContents = function(node) {
        var d = this.contentDocument;
        d.selection.empty();
        var range = d.body.createTextRange();
        range.moveToElementText(node);
        range.select();
    };
    prototype.selectRange = function(start, startOffset, end, endOffset) {
        var d = this.contentDocument;
        var body = d.body;
        d.selection.empty();
        var range = endPoint(start, startOffset);
        if (start != end || startOffset != endOffset) {
            range.setEndPoint("EndToEnd", endPoint(end, endOffset));
        }
        range.select();

        function endPoint(node, offset) {
            var range;
            if (node.nodeType == 1) {
                var childNodes = node.childNodes;
                if (offset >= childNodes.length) {
                    range = body.createTextRange();
                    range.moveToElementText(node);
                    range.collapse(false);
                    return range;
                }
                node = childNodes[offset];
                if (node.nodeType == 1) {
                    range = body.createTextRange();
                    range.moveToElementText(node);
                    range.collapse(true);
                    switch (node.tagName.toLowerCase()) {
                    case "table":
                        range.move("character", -1);
                        break;
                    }
                    return range;
                }
                return endPoint(node, 0);
            }
            if (node.nodeType != 3) {
                throw "selectRange: nodeType != @".replace(/@/, node.nodeType);
            }

            range = body.createTextRange();
            var element = node.previousSibling;
            while (element) {
                var nodeType = element.nodeType;
                if (nodeType == 1) {
                    range.moveToElementText(element);
                    range.collapse(false);
                    break;
                }
                if (nodeType == 3) {
                    offset += element.nodeValue.length;
                }
                element = element.previousSibling;
            }
            if (!element) {
                range.moveToElementText(node.parentNode);
                range.collapse(true);
            }
            if (offset != 0) {
                range.move("character", offset);
            }
            return range;
        }
    };
    prototype.getSelectionRange = function() {
        var body = this.contentDocument.body;
        var pseudo = {};
        var start = this.getNativeSelectionRange();
        if (start.item) {
            var element = start.item(0);
            var parent = element.parentNode;
            var childNodes = parent.childNodes;
            var length = childNodes.length;
            for (var i = 0; i < length; i++) {
                if (childNodes[i] == element) {
                    pseudo.startOffset = i;
                    pseudo.endOffset = i + 1;
                    break;
                }
            }
            pseudo.collapsed = false;
            pseudo.startContainer = pseudo.endContainer = parent;
            return pseudo;
        }
        var end = start.duplicate();
        pseudo.collapsed = start.compareEndPoints("StartToEnd", end) == 0;
        start.collapse(true);
        end.collapse(false);

        function nextElement(range) {
            var parent = range.parentElement();
            var childNodes = parent.childNodes;
            var length = childNodes.length;
            for (var i = 0; i < length; i++) {
                var node = childNodes[i];
                if (node.nodeType == 1) {
                    var tmp = body.createTextRange();
                    tmp.moveToElementText(node);
                    if (range.compareEndPoints("EndToStart", tmp) <= 0) {
                        return node;
                    }
                }
            }
            return null;
        }

        function nodeOffset(range, parent, element, index, length) {
            var tmp = body.createTextRange();
            tmp.moveToElementText(element || parent);
            tmp.collapse(!!element);
            tmp.move("character", -index);
            if (!element) {
                length++;
            }
            for ( ; length >= 0; length--) {
                if (tmp.compareEndPoints("EndToStart", range) == 0) {
                    return length;
                }
                tmp.move("character", -1);
            }
            return null;
        }

        function setContainerOffset(range, containerKey, offsetKey) {
            var parent = range.parentElement();
            var element = nextElement(range);
            var index = 0;
            var node = element ? element.previousSibling : parent.lastChild;
            var offset, length;
            while (node && node.nodeType == 3) {
                length = node.nodeValue.length;
                offset = nodeOffset(range, parent, element, index, length);
                if (offset !== null) {
                    pseudo[containerKey] = node;
                    pseudo[offsetKey] = offset;
                    return;
                }
                index += length;
                node = node.previousSibling;
            }
            var childNodes = parent.childNodes;
            length = childNodes.length;
            if (length > 0) {
                pseudo[containerKey] = parent;
                pseudo[offsetKey] = containerKey == "startContainer" ? 0 : length - 1;
                return;
            }
            element = parent;
            parent = element.parentNode;
            childNodes = parent.childNodes;
            length = childNodes.length;
            for (offset = 0; offset < length; offset++) {
                if (element == childNodes[offset]) {
                    pseudo[containerKey] = parent;
                    pseudo[offsetKey] = offset;
                    return;
                }
            }
        }

        setContainerOffset(start, "startContainer", "startOffset");
        setContainerOffset(end, "endContainer", "endOffset");
        return pseudo;
    };
    prototype.getNativeSelectionRange = function() {
        this.contentWindow.focus();
        return this.contentDocument.selection.createRange();
    };
    prototype.getSelectionText = function() {
        var range = this.getNativeSelectionRange();
        if (range) {
            return range.item ? range.item(0).innerText : range.text;
        }
        return null;
    };
    prototype.getSelectionHTML = function() {
        var range = this.getNativeSelectionRange();
        if (range) {
            return range.item ? range.item(0).innerHTML : range.htmlText;
        }
        return null;
    };
    prototype.getSelectionFragment = function() {
        var d = this.contentDocument;
        var fragment = d.createDocumentFragment();
        var anonymous = d.createElement("div");
        anonymous.innerHTML = this.getSelectionHTML();
        this.collectChildNodes(fragment, anonymous);
        return fragment;
    };
    prototype.getSelectionPosition = function() {
        this.contentWindow.focus();
        var d = this.contentDocument;
        var range = d.selection.createRange();
        var startNode = null;
        var endNode = null;
        if (range.item) {
            if (range.item(0).ownerDocument == d) {
                startNode = range.item(0);
                endNode = range.item(range.length - 1);
            }
        }
        else {
            if (range.parentElement().ownerDocument == d) {
                var startRange = range.duplicate();
                startRange.collapse(true);
                startNode = startRange.parentElement();
                var endRange = range.duplicate();
                endRange.collapse(false);
                endNode = endRange.parentElement();
            }
        }
        return { start: startNode, end: endNode };
    };
    prototype.expandSelectionToElement = function(arg) {
        this.contentWindow.focus();
        var d = this.contentDocument;
        var body = d.body;
        var range = d.selection.createRange();
        var tmp;
        if (arg.start) {
            tmp = body.createTextRange();
            tmp.moveToElementText(arg.start);
            range.setEndPoint("StartToStart", tmp);
        }
        if (arg.end) {
            tmp = body.createTextRange();
            tmp.moveToElementText(arg.end);
            range.setEndPoint("EndToEnd", tmp);
        }
        if (tmp) {
            range.select();
        }
    };
    prototype.selectionContainsTagName = function(name) {
        this.contentWindow.focus();
        var d = this.contentDocument;
        var selection = d.selection;
        var range = selection.createRange();
        var parent = range.item ? range.item(0) : range.parentElement();
        if (!parent) {
            return false;
        }
        if (TracWysiwyg.getSelfOrAncestor(parent, name)) {
            return true;
        }
        var elements = parent.getElementsByTagName(name);
        var length = elements.length;
        for (var i = 0; i < length; i++) {
            var testRange = selection.createRange();
            testRange.moveToElementText(elements[i]);
            if (range.compareEndPoints("StartToEnd", testRange) <= 0
                && range.compareEndPoints("EndToStart", testRange) >= 0)
            {
                return true;
            }
        }
        return false;
    };
    prototype.insertHTML = prototype._msieInsertHTML;
}
else {
    prototype.appendBogusLineBreak = function(element) { };
    prototype.insertParagraphOnEnter = null;
    prototype.insertLineBreak = function() { };
    prototype.insertTableCell = function(row, index) { return null };
    prototype.getFocusNode = function() { return null };
    prototype.selectNode = function(node) { };
    prototype.selectNodeContents = function(node) { return null };
    prototype.selectRange = function(start, startOffset, end, endOffset) { };
    prototype.getSelectionRange = function() { return null };
    prototype.getNativeSelectionRange = function() { return null };
    prototype.getSelectionText = function() { return null };
    prototype.getSelectionHTML = function() { return null };
    prototype.getSelectionFragment = function() { return null };
    prototype.getSelectionPosition = function() { return null };
    prototype.expandSelectionToElement = function(arg) { };
    prototype.selectionContainsTagName = function(name) { return false };
    prototype.insertHTML = function(html) { };
}

prototype._treeWalkEmulation = function(root, iterator) {
    if (!root.firstChild) {
        iterator(null);
        return;
    }
    var element = root;
    var tmp;
    while (element) {
        if (tmp = element.firstChild) {
            element = tmp;
        }
        else if (tmp = element.nextSibling) {
            element = tmp;
        }
        else {
            for ( ; ; ) {
                element = element.parentNode;
                if (element == root || !element) {
                    iterator(null);
                    return;
                }
                if (tmp = element.nextSibling) {
                    element = tmp;
                    break;
                }
            }
        }
        iterator(element);
    }
};

if (document.createTreeWalker) {
    prototype.treeWalk = function(root, iterator) {
        var walker = root.ownerDocument.createTreeWalker(
            root, NodeFilter.SHOW_ELEMENT | NodeFilter.SHOW_TEXT, null, true);
        while (walker.nextNode()) {
            iterator(walker.currentNode);
        }
        iterator(null);
    };
}
else {
    prototype.treeWalk = prototype._treeWalkEmulation;
}

TracWysiwyg.instances = [];
TracWysiwyg.tracPaths = null;

TracWysiwyg.newInstance = function(textarea, options) {
    var instance = new TracWysiwyg(textarea, options);
    TracWysiwyg.instances.push(instance);
    return instance;
};

TracWysiwyg.findInstance = function(textarea) {
    var instances = TracWysiwyg.instances;
    var length = instances.length;
    for (var i = 0; i < length; i++) {
        var instance = instances[i];
        if (instance.textarea == textarea) {
            return instance;
        }
    }
    return null;
};

TracWysiwyg.getTracPaths = function() {
    var stylesheets = [];
    var paths = { stylesheets: stylesheets };

    var head = document.getElementsByTagName("head")[0];
    var links = head.getElementsByTagName("link");
    var length = links.length;
    for (var i = 0; i < length; i++) {
        var link = links[i];
        var href = link.getAttribute("href") || "";
        var type = link.getAttribute("type") || "";
        switch ((link.getAttribute("rel") || "").toLowerCase()) {
        case "tracwysiwyg.base":
            paths.base = href;
            break;
        case "tracwysiwyg.stylesheet":
            stylesheets.push(href);
            break;
        }
    }
    if (paths.base && stylesheets.length > 0) {
        return paths;
    }
    return null;
};

TracWysiwyg.getOptions = function() {
    return window._tracwysiwyg || {};
};

TracWysiwyg.getEditorMode = function() {
    if (TracWysiwyg.editorMode) {
        return TracWysiwyg.editorMode;
    }

    var mode = null;
    var cookies = (document.cookie || "").split(";");
    var length = cookies.length;
    for (var i = 0; i < length; i++) {
        var match = /^\s*tracwysiwyg=(\S*)/.exec(cookies[i]);
        if (match) {
            switch (match[1]) {
            case "wysiwyg":
                mode = match[1];
                break;
            default:    // "textarea"
                mode = null;
                break;
            }
            break;
        }
    }

    TracWysiwyg.editorMode = mode || "textarea";
    return TracWysiwyg.editorMode;
};

TracWysiwyg.setEditorMode = function(mode) {
    switch (mode) {
    case "wysiwyg":
        break;
    default:    // "textarea"
        mode = "textarea";
        break;
    }
    TracWysiwyg.editorMode = mode;

    var now = new Date();
    if (!/\/$/.test(TracWysiwyg.tracPaths.base)) {
        expires = new Date(now.getTime() - 86400000);
        pieces = [ "tracwysiwyg=",
            "path=" + TracWysiwyg.tracPaths.base + "/",
            "expires=" + expires.toUTCString() ];
        document.cookie = pieces.join("; ");
    }
    var expires = new Date(now.getTime() + 365 * 86400 * 1000);
    var pieces = [ "tracwysiwyg=" + mode,
        "path=" + TracWysiwyg.tracPaths.base,
        "expires=" + expires.toUTCString() ];
    document.cookie = pieces.join("; ");
};

TracWysiwyg.stopEvent = function(event) {
    if (event.preventDefault) {
        event.preventDefault();
        event.stopPropagation();
    }
    else {
        event.returnValue = false;
        event.cancelBubble = true;
    }
};

TracWysiwyg.setStyle = function(element, object) {
    var style = element.style;
    for (var name in object) {
        style[name] = object[name];
    }
};

if (document.defaultView) {
    TracWysiwyg.getStyle = function(element, name) {
        var value = element.style[name];
        if (!value) {
            var style = element.ownerDocument.defaultView.getComputedStyle(element, null)
            value = style ? style[name] : null;
        }
        return value;
    };
}
else {
    TracWysiwyg.getStyle = function(element, name) {
        return element.style[name] || element.currentStyle[name];
    };
}

TracWysiwyg.elementPosition = function(element) {
    function vector(left, top) {
        var value = [ left, top ];
        value.left = left;
        value.top = top;
        return value;
    }
    var position = TracWysiwyg.getStyle(element, "position");
    var left = 0, top = 0;
    for (var node = element; node; node = node.offsetParent) {
        left += node.offsetLeft || 0;
        top += node.offsetTop || 0;
    }
    if (position != "absolute") {
        return vector(left, top);
    }
    var offset = TracWysiwyg.elementPosition(element.offsetParent);
    return vector(left - offset.left, top - offset.top);
};

TracWysiwyg.getSelfOrAncestor = function(element, name) {
    var target = element;
    var d = element.ownerDocument;
    if (name instanceof RegExp) {
        while (target && target != d) {
            switch (target.nodeType) {
            case 1: // element
                if (name.test(target.tagName.toLowerCase())) {
                    return target;
                }
                break;
            case 11: // fragment
                return null;
            }
            target = target.parentNode;
        }
    }
    else {
        name = name.toLowerCase();
        while (target && target != d) {
            switch (target.nodeType) {
            case 1: // element
                if (target.tagName.toLowerCase() == name) {
                    return target;
                }
                break;
            case 11: // fragment
                return null;
            }
            target = target.parentNode;
        }
    }
    return null;
};

TracWysiwyg.serializeToHref = function(attrs) {
    var texts = [];
    for (var name in attrs) {
        if (/^data(?:-|$)/.exec(name)) {
            texts.push(encodeURIComponent(name) + "=" + encodeURIComponent(attrs[name]));
        }
    }
    return "#" + texts.join("&");
};

TracWysiwyg.unserializeFromHref = function(href, name) {
    var attrs = {};
    if (href.indexOf("#") !== -1) {
        var pieces = href.replace(/^[^#]*#/, '').split(/&/g);
        var length = pieces.length;
        for (var i = 0; i < length; i++) {
            var pair = pieces[i].split(/=/g, 2);
            attrs[decodeURIComponent(pair[0])] = decodeURIComponent(pair[1]);
        }
    }
    return name ? attrs[name] : attrs;
};

TracWysiwyg.getTextContent = (function() {
    var anonymous = document.createElement("div");
    if (typeof anonymous.textContent != "undefined") {
        return function(element) { return element.textContent };
    }
    else if (typeof anonymous.innerText != "undefined") {
        return function(element) { return element.innerText };
    }
    else {
        return function(element) { return null };
    }
})();

TracWysiwyg.initialize = function() {
    if ("replace".replace(/[a-e]/g, function(m) { return "*" }) != "r*pl***") {
        return false;
    }
    if (typeof document.designMode == "undefined") {
        return false;
    }
    TracWysiwyg.tracPaths = TracWysiwyg.getTracPaths();
    if (!TracWysiwyg.tracPaths) {
        return false;
    }
    var options = TracWysiwyg.getOptions();
    var textareas = document.getElementsByTagName("textarea");
    for (var i = 0; i < textareas.length; i++) {
        var textarea = textareas[i];
        if (/\bwikitext\b/.test(textarea.className || "") &&
            textarea.getAttribute('data-tracwysiwyg-initialized') === null)
        {
            TracWysiwyg.newInstance(textarea, options);
        }
    }
    return true;
};

window.TracWysiwyg = TracWysiwyg;

if (window._tracwysiwyg !== undefined) {
    $(document).ready(function($) {
        setTimeout(TracWysiwyg.initialize, 10);
    });
}

})(jQuery, window, document);
