// Injected first by esbuild — exposes React as globals so JSX files can use
// React.useState / React.useEffect etc. without importing.
import * as React from 'react';
import * as ReactDOM from 'react-dom/client';
globalThis.React = React;
globalThis.ReactDOM = ReactDOM;
