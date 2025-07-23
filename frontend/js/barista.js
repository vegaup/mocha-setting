const Barista = {
    send: (action, data = {}) => {
        const message = { action, ...data };
        if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.mochaBridge) {
            window.webkit.messageHandlers.mochaBridge.postMessage(JSON.stringify(message));
        } else {
            console.error("Barista bridge not available.");
        }
    }
};
