module.exports = [
    { type: "heading", defaultValue: "Dress Watch Configuration" },
    { type: "toggle", messageKey: "ShowDate",      label: "Show Date",       defaultValue: false },
    { type: "toggle", messageKey: "ShowBattery",   label: "Show Battery",    defaultValue: false },
    {
        type: "section",
        items: [
            { type: "heading", defaultValue: "Font" },
            { type: "toggle", messageKey: "UseBoldFont",   label: "Use Bold Font",   defaultValue: false },
            { type: "toggle", messageKey: "UseLargerFont", label: "Use Larger Font", defaultValue: false }
        ]
    },
    { type: "submit", defaultValue: "Save Settings" }
];
