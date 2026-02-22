import {
    Bluetooth,
    Shield,
    Smartphone,
    Lock,
    KeySquareIcon,
    Zap,
    Github,
    Feather,
    Fingerprint,
} from "lucide-react";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import {
    Card,
    CardContent,
    CardDescription,
    CardHeader,
    CardTitle,
} from "@/components/ui/card";
import { Separator } from "@/components/ui/separator";

const NAV_LINKS = ["How It Works", "Security", "Hardware", "App"];

const HOW_IT_WORKS_STEPS = [
    {
        step: "01",
        icon: <KeySquareIcon className="h-5 w-5" />,
        title: "Plug It In",
        description:
            "Connect keypr to your computer via USB. The device powers on and registers as a FIDO2 hardware key.",
    },
    {
        step: "02",
        icon: <Lock className="h-5 w-5" />,
        title: "Passkey Prompt",
        description:
            "When a site requests a passkey, keypr intercepts the challenge and holds it. Nothing is signed yet.",
    },
    {
        step: "03",
        icon: <Smartphone className="h-5 w-5" />,
        title: "Push to Your Phone",
        description:
            "keypr instantly sends a push notification to your paired phone. Your phone must be connected via Bluetooth to proceed.",
    },
    {
        step: "04",
        icon: <Bluetooth className="h-5 w-5" />,
        title: "Approve on Bluetooth",
        description:
            "Tap accept on the notification. Your phone confirms over the encrypted BLE channel, authorizing the device to sign.",
    },
    {
        step: "05",
        icon: <Zap className="h-5 w-5" />,
        title: "Passkey Goes Through",
        description:
            "keypr signs the challenge and completes the authentication.",
    },
];

const SECURITY_FEATURES = [
    {
        icon: <Shield className="h-6 w-6" />,
        title: "Keys Never Leave the Device",
        description:
            "Passkey private keys are stored on the ESP32-S3 and never transmitted in plaintext.",
    },
    {
        icon: <Bluetooth className="h-6 w-6" />,
        title: "Encrypted BLE Channel",
        description:
            "All communication between the device and your phone uses an encrypted BLE pairing channel with a shared secret.",
    },
    {
        icon: <Fingerprint className="h-6 w-6" />,
        title: "App Authentication",
        description:
            "Phone verification via Face ID, Touch ID, or PIN acts as an added layer of security.",
    },
    {
        icon: <Github className="h-6 w-6" />,
        title: "Open Source",
        description:
            "All firmware, hardware schematics, and app code are public.",
    },
    {
        icon: <Zap className="h-6 w-6" />,
        title: "Completely Offline",
        description:
            "keypr requires no server, no account, and no internet connection. Everything stays local between your device and phone.",
    },
    {
        icon: <Lock className="h-6 w-6" />,
        title: "Phone & Device Required",
        description:
            "The USB device alone cannot sign anything. And without Bluetooth approval from your paired phone, it cannot authenticate.",
    },
];

const HARDWARE_SPECS = [
    { label: "Microcontroller", value: "ESP32-S3" },
    { label: "Connectivity", value: "Bluetooth Low Energy (BLE)" },
    { label: "Firmware", value: "ESP-IDF (C/C++)" },
    { label: "Interface", value: "USB (FIDO HID / CCID)" },
    { label: "Auth Standard", value: "FIDO 2.1 / CTAP 2" },
    { label: "Form Factor", value: "USB Dongle" },
];

const APP_FEATURES = [
    "BLE device pairing and management",
    "Biometric / PIN unlock to authorize device",
    "View stored passkey sites",
    "Remote device lock",
];

const App = () => {
    return (
        <div className="min-h-screen bg-k-bg text-k-silver antialiased">
            {/* Nav */}
            <header className="relative z-10 border-b border-k-silver/10 font-bold">
                <div className="mx-auto max-w-6xl px-6 py-4 flex items-center justify-between">
                    <div className="flex items-center gap-2">
                        <KeySquareIcon className="h-5 w-5 text-k-gold" />
                        <span className="text-lg font-normal font-kyiv-medium tracking-tight text-k-gold">
                            keypr
                        </span>
                    </div>
                    <nav className="hidden md:flex items-center gap-10">
                        {NAV_LINKS.map((link) => (
                            <a
                                key={link}
                                href={`#${link.toLowerCase().replace(/\s+/g, "-")}`}
                                className="text-sm text-k-silver hover:text-k-silver/60 transition-colors"
                            >
                                {link}
                            </a>
                        ))}
                    </nav>
                    <div className="flex items-center gap-3">
                        <Button variant="ghost" size="sm" asChild>
                            <a
                                href="https://github.com/tylerrlin/keypr"
                                target="_blank"
                                rel="noopener noreferrer"
                                className="flex items-center gap-2"
                            >
                                <Github className="h-5 w-5" />
                                <span className="hidden sm:inline font-bold text-sm">
                                    GitHub
                                </span>
                            </a>
                        </Button>
                    </div>
                </div>
            </header>

            {/* Hero */}
            <section className="relative z-10 mx-auto max-w-6xl px-6 pt-28 pb-32 text-center">
                <h1 className="mx-auto max-w-3xl text-5xl font-kyiv-heavy tracking-tight leading-tight sm:text-6xl lg:text-7xl text-k-gold">
                    Proximity-Locked Passkey Vault
                </h1>
                <p className="mx-auto mt-6 max-w-xl text-lg text-k-silver/70 leading-relaxed">
                    <span className="font-normal font-kyiv-medium text-k-gold">
                        keypr
                    </span>{" "}
                    stores your FIDO2 passkeys on an{" "}
                    <span className="text-k-silver">ESP32-S3</span> device. They
                    only unlock when your phone is nearby.
                </p>
                <div className="mt-10 flex flex-col sm:flex-row items-center justify-center gap-4">
                    <Button variant="outline" size="lg" asChild>
                        <a
                            href="https://github.com/tylerrlin/keypr"
                            target="_blank"
                            rel="noopener noreferrer"
                            className="flex items-center gap-2"
                        >
                            <Github className="h-4 w-4" />
                            View on GitHub
                        </a>
                    </Button>
                </div>

                {/* Status pills */}
                <div className="mt-16 flex flex-wrap justify-center gap-3 font-bold">
                    {[
                        {
                            icon: <Shield className="h-3.5 w-3.5" />,
                            label: "Encrypted Passkey Store",
                        },
                        {
                            icon: <Bluetooth className="h-3.5 w-3.5" />,
                            label: "BLE (Bluetooth Low Energy)",
                        },
                        {
                            icon: <Zap className="h-3.5 w-3.5" />,
                            label: "FIDO / CTAP2",
                        },
                    ].map(({ icon, label }) => (
                        <div
                            key={label}
                            className="flex items-center gap-2 rounded-full border border-k-blue/30 bg-k-blue/10 px-4 py-1.5 text-xs text-k-blue"
                        >
                            {icon}
                            {label}
                        </div>
                    ))}
                </div>
            </section>

            <Separator />

            {/* How It Works */}
            <section
                id="how-it-works"
                className="relative z-10 mx-auto max-w-6xl px-6 py-28"
            >
                <div className="mb-14 text-center">
                    <Badge variant="outline" className="mb-4">
                        How It Works
                    </Badge>
                    <h2 className="text-3xl font-bold tracking-tight sm:text-4xl text-k-gold ">
                        Better authentication
                    </h2>
                    <p className="mt-3 text-k-silver/60 max-w-lg mx-auto">
                        Physical possession and phone verification are required
                        to unlock your passkeys.
                    </p>
                </div>

                <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-3">
                    {HOW_IT_WORKS_STEPS.map((step, i) => (
                        <Card
                            key={step.step}
                            className={
                                i === 4 ? "sm:col-span-2 lg:col-span-1" : ""
                            }
                        >
                            <CardHeader>
                                <div className="flex items-center gap-3 mb-1">
                                    <span className="text-xs font-mono text-k-silver/30">
                                        {step.step}
                                    </span>
                                    <div className="flex h-8 w-8 items-center justify-center rounded-lg bg-k-gold/10 text-k-gold">
                                        {step.icon}
                                    </div>
                                </div>
                                <CardTitle className="text-base">
                                    {step.title}
                                </CardTitle>
                            </CardHeader>
                            <CardContent>
                                <CardDescription>
                                    {step.description}
                                </CardDescription>
                            </CardContent>
                        </Card>
                    ))}
                </div>
            </section>

            <Separator />

            {/* Security */}
            <section
                id="security"
                className="relative z-10 mx-auto max-w-6xl px-6 py-28"
            >
                <div className="mb-14 text-center">
                    <Badge variant="outline" className="mb-4">
                        Security Model
                    </Badge>
                    <h2 className="text-3xl font-bold tracking-tight sm:text-4xl text-k-gold">
                        Security-first design
                    </h2>
                    <p className="mt-3 text-k-silver/60 max-w-lg mx-auto">
                        Every layer assumes the previous one can be compromised.
                    </p>
                </div>

                <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-3">
                    {SECURITY_FEATURES.map((feature) => (
                        <Card key={feature.title}>
                            <CardHeader>
                                <div className="mb-2 flex h-10 w-10 items-center justify-center rounded-xl bg-k-blue/10 text-k-blue">
                                    {feature.icon}
                                </div>
                                <CardTitle className="text-base">
                                    {feature.title}
                                </CardTitle>
                            </CardHeader>
                            <CardContent>
                                <CardDescription>
                                    {feature.description}
                                </CardDescription>
                            </CardContent>
                        </Card>
                    ))}
                </div>
            </section>

            <Separator />

            {/* Hardware */}
            <section
                id="hardware"
                className="relative z-10 mx-auto max-w-6xl px-6 py-28"
            >
                <div className="grid grid-cols-1 gap-16 lg:grid-cols-2 lg:items-center">
                    <div>
                        <Badge variant="outline" className="mb-4">
                            Hardware
                        </Badge>
                        <h2 className="text-3xl font-bold tracking-tight sm:text-4xl text-k-gold">
                            Built on the ESP32-S3
                        </h2>
                        <p className="mt-4 text-k-silver/70 leading-relaxed">
                            All that's needed is an ESP32-S3, which provides{" "}
                            <span className="font-kyiv-medium text-k-gold">
                                keypr
                            </span>{" "}
                            with native USB and Bluetooth Low Energy Support
                            without any additional chips. The device runs custom
                            firmware built on top of the ESP-IDF, and all
                            schematics and code are open source.
                        </p>
                        <div className="mt-8 flex items-center gap-3">
                            <div className="flex h-12 w-12 items-center justify-center rounded-xl border border-k-blue/20 bg-k-blue/10">
                                <Feather className="h-6 w-6 text-k-blue" />
                            </div>
                            <div>
                                <p className="text-sm font-medium text-k-silver">
                                    Lightweight and low-cost
                                </p>
                                <p className="text-xs text-k-silver/50">
                                    Easily accessible and affordable
                                </p>
                            </div>
                        </div>
                    </div>

                    <Card>
                        <CardHeader>
                            <CardTitle className="text-sm font-mono text-k-gold/50 uppercase tracking-widest">
                                Specifications
                            </CardTitle>
                        </CardHeader>
                        <CardContent className="space-y-0">
                            {HARDWARE_SPECS.map((spec, i) => (
                                <div key={spec.label}>
                                    <div className="flex items-center justify-between py-3">
                                        <span className="text-sm text-k-silver/50">
                                            {spec.label}
                                        </span>
                                        <span className="text-sm font-medium text-k-silver">
                                            {spec.value}
                                        </span>
                                    </div>
                                    {i < HARDWARE_SPECS.length - 1 && (
                                        <Separator />
                                    )}
                                </div>
                            ))}
                        </CardContent>
                    </Card>
                </div>
            </section>

            <Separator />

            {/* App */}
            <section
                id="app"
                className="relative z-10 mx-auto max-w-6xl px-6 py-28"
            >
                <div className="grid grid-cols-1 gap-16 lg:grid-cols-2 lg:items-center">
                    {/* Feature list */}
                    <Card className="order-2 lg:order-1">
                        <CardHeader>
                            <CardTitle className="text-sm font-mono text-k-gold/50 uppercase tracking-widest">
                                App Features
                            </CardTitle>
                        </CardHeader>
                        <CardContent className="space-y-3">
                            {APP_FEATURES.map((feature) => (
                                <div
                                    key={feature}
                                    className="flex items-start gap-3"
                                >
                                    <div className="mt-0.5 flex h-5 w-5 shrink-0 items-center justify-center rounded-full border border-k-gold/30 bg-k-gold/5">
                                        <div className="h-1.5 w-1.5 rounded-full bg-k-gold" />
                                    </div>
                                    <span className="text-sm text-k-silver/70">
                                        {feature}
                                    </span>
                                </div>
                            ))}
                        </CardContent>
                    </Card>

                    {/* Text */}
                    <div className="order-1 lg:order-2">
                        <Badge variant="outline" className="mb-4">
                            Companion App
                        </Badge>
                        <h2 className="text-3xl font-bold tracking-tight sm:text-4xl text-k-gold">
                            Phone Authentication
                        </h2>
                        <p className="mt-4 text-k-silver/70 leading-relaxed">
                            Verifies your identity using your phone's
                            authentication system before interfacing with the
                            device. Keeps your passkeys both secure and
                            accessible.
                        </p>
                        <div className="mt-8 flex gap-3">
                            <div className="relative group">
                                <Button
                                    variant="outline"
                                    className="gap-2 hover:cursor-pointer"
                                >
                                    <Smartphone className="h-4 w-4" />
                                    iOS App Store
                                </Button>
                                <div className="absolute -bottom-8 -right-9 -translate-x-1/2 px-2.5 py-1 rounded-md bg-k-bg border border-k-silver/20 text-xs text-k-silver/60 whitespace-nowrap opacity-0 group-hover:opacity-100 transition-opacity pointer-events-none">
                                    Coming soon
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </section>

            <Separator />

            {/* Footer */}
            <footer className="relative z-10 border-t border-k-silver/10">
                <div className="mx-auto max-w-6xl px-6 py-8 flex flex-col sm:flex-row items-center justify-between gap-4">
                    <div className="flex items-center gap-2 text-k-gold/60">
                        <KeySquareIcon className="h-4 w-4" />
                        <span className="text-sm font-kyiv-medium font-bold">
                            keypr
                        </span>
                    </div>
                    <p className="text-xs text-k-silver/30">
                        Open source · ESP32-S3
                    </p>
                    <a
                        href="https://github.com/tylerrlin/keypr"
                        target="_blank"
                        rel="noopener noreferrer"
                        className="flex items-center gap-1.5 text-xs text-k-silver/30 hover:text-k-silver transition-colors"
                    >
                        <Github className="h-3.5 w-3.5" />
                        GitHub
                    </a>
                </div>
            </footer>
        </div>
    );
};

export default App;
