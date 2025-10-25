#include "Pitchblade/effects/FormantShifter.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    inline bool isFinite5(float a, float b, float c, float d, float e) noexcept
    {
        return std::isfinite(a) && std::isfinite(b) && std::isfinite(c)
            && std::isfinite(d) && std::isfinite(e);
    }

    inline float tinyDenormGuard(float v) noexcept
    {
        // kill subnormals cheaply
        if (std::abs(v) < 1e-30f) return 0.0f;
        return v;
    }
}

struct FormantShifter::Impl
{
    static constexpr int maxFormants = 2;
    static constexpr int maxChannels = 2;

    // helpers
    static float amountToRatio(float a) noexcept
    {
        const float t = juce::jlimit(-1.0f, 1.0f, a / 50.0f); // ±2 octaves on the UI
        return std::pow(2.0f, 2.0f * t);                      // 0.25x..4x
    }

    static float clampHz(float f, float nyq) noexcept
    {
        return juce::jlimit(80.0f, nyq - 500.0f, f);
    }

    static float pickQ(float fHz) noexcept
    {
        if (fHz < 700.f)  return 2.8f;
        if (fHz < 1500.f) return 3.6f;
        return 4.8f;
    }

    static inline float dbToLin(float dB) noexcept
    {
        return juce::Decibels::decibelsToGain(dB);
    }

    // small smoothed band-pass (RBJ) with smoothed coeffs
    struct SmoothBandpass
    {
        // target params
        float fT = 1000.f, qT = 3.6f, gT_db = -120.f;
        // current (smoothed)
        float fC = 1000.f, qC = 3.6f, gC_db = -120.f;

        // biquad coeffs (current/target)
        float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float b0T = 0, b1T = 0, b2T = 0, a1T = 0, a2T = 0;

        // DF2 state
        float s1 = 0, s2 = 0;

        // smoothing rates
        float alphaParam = 0.f; // ~5–10 ms
        float alphaCoeff = 0.f; // ~5–10 ms
        float fs = 48000.f;

        static float msToAlpha(float ms, float sampleRate)
        {
            const float tau = std::max(0.0001f, ms * 0.001f);
            return 1.f - std::exp(-1.f / (tau * sampleRate));
        }

        void setSmoothingMs(float paramMs, float coeffMs, float sampleRate)
        {
            fs = sampleRate;
            alphaParam = msToAlpha(std::max(0.5f, paramMs), sampleRate);
            alphaCoeff = msToAlpha(std::max(0.5f, coeffMs), sampleRate);
        }

        void setTargets(float fHz, float q, float gDb) noexcept
        {
            // light sanity clamps since these can come from outside
            fT    = std::isfinite(fHz) ? fHz : 1000.f;
            qT    = juce::jlimit(0.25f, 24.0f, std::isfinite(q) ? q : 3.6f);
            gT_db = juce::jlimit(-120.0f, 12.0f, std::isfinite(gDb) ? gDb : -120.0f);
        }

        void reset()
        {
            s1 = s2 = 0.f;
            fC = fT; qC = qT; gC_db = gT_db;
            recalcTargets();
            b0 = b0T; b1 = b1T; b2 = b2T; a1 = a1T; a2 = a2T;
        }

        void recalcTargets()
        {
            // if anything went bad, snap to a safe mute
            if (!isFinite5(fC, qC, gC_db, fs, 1.f))
            {
                b0T = b1T = b2T = a1T = a2T = 0.f;
                return;
            }

            const float fSafe = juce::jlimit(60.f, fs * 0.45f, fC);
            const float w0 = juce::MathConstants<float>::twoPi * fSafe / fs;
            const float c  = std::cos(w0);
            const float s  = std::sin(w0);
            const float qSafe = std::max(0.25f, qC);
            const float alpha = s / (2.f * qSafe);

            // RBJ band-pass (constant skirt gain)
            const float b0n = qSafe * alpha;
            const float b1n = 0.f;
            const float b2n = -qSafe * alpha;
            const float a0n = 1.f + alpha;
            const float a1n = -2.f * c;
            const float a2n = 1.f - alpha;

            const float invA0 = (std::abs(a0n) > 1e-12f ? 1.f / a0n : 0.f);
            b0T = b0n * invA0; b1T = b1n * invA0; b2T = b2n * invA0;
            a1T = a1n * invA0; a2T = a2n * invA0;

            if (!isFinite5(b0T, b1T, b2T, a1T, a2T))
                b0T = b1T = b2T = a1T = a2T = 0.f;
        }

        float process(float x) noexcept
        {
            // smooth params
            fC    += alphaParam * (fT    - fC);
            qC    += alphaParam * (qT    - qC);
            gC_db += alphaParam * (gT_db - gC_db);

            // update coeffs
            recalcTargets();
            b0 += alphaCoeff * (b0T - b0);
            b1 += alphaCoeff * (b1T - b1);
            b2 += alphaCoeff * (b2T - b2);
            a1 += alphaCoeff * (a1T - a1);
            a2 += alphaCoeff * (a2T - a2);

            // if anything went non-finite, hard-mute this band until reset by caller
            if (!isFinite5(b0, b1, b2, a1, a2))
                return 0.f;

            // DF2
            const float y = b0 * x + s1;
            s1 = tinyDenormGuard(b1 * x - a1 * y + s2);
            s2 = tinyDenormGuard(b2 * x - a2 * y);

            const float g = juce::Decibels::decibelsToGain(gC_db);
            return std::isfinite(g) ? y * g : 0.f;
        }
    };

    struct ChannelState
    {
        std::array<SmoothBandpass, maxFormants> bands{};
        int nActive = 0;

        // tiny air passthrough (HP)
        float hp_y = 0.f, hp_x1 = 0.f, hp_a = 0.f;

        // light speech bed (HP ~120 Hz, LP ~12 kHz)
        float sp_hp_y = 0.f, sp_hp_x1 = 0.f, sp_hp_a = 0.f;
        float sp_lp_y = 0.f,            sp_lp_a = 0.f;

        juce::LinearSmoothedValue<float> outTrim { 1.f };

        void reset()
        {
            nActive = 0;
            hp_y = hp_x1 = 0.f;
            sp_hp_y = sp_hp_x1 = sp_lp_y = 0.f;
            outTrim.setCurrentAndTargetValue(1.f);
            for (auto& b : bands) b.reset();
        }
    };

    // state
    double sr = 48000.0;
    int channels = 1;

    std::array<std::vector<float>, maxChannels> lastGoodFreqs{};
    juce::LinearSmoothedValue<float> amountSm { 0.f };
    juce::LinearSmoothedValue<float> mixSm    { 1.f };
    std::array<ChannelState, maxChannels> ch{};

    // temp buffer reused to avoid heap allocs in processBlock
    juce::AudioBuffer<float> dryTmp;

    void prepare(double sampleRate, int /*maxBlockSize*/, int numChannels)
    {
        sr = sampleRate;
        channels = juce::jlimit(1, maxChannels, numChannels);

        amountSm.reset(sr, 0.004); amountSm.setCurrentAndTargetValue(0.f);
        mixSm   .reset(sr, 0.006); mixSm   .setCurrentAndTargetValue(1.f);

        for (int ci = 0; ci < maxChannels; ++ci)
        {
            ch[ci].reset();
            ch[ci].outTrim.reset(sr, 0.015);

            for (auto& b : ch[ci].bands)
            {
                b.setSmoothingMs(6.f, 9.f, (float) sr);
                b.setTargets(1000.f, 3.6f, -120.f);
                b.reset();
            }

            // HP ~3 kHz for tiny "air" passthrough
            const float fc = 3000.f;
            const float k  = std::exp(-juce::MathConstants<float>::twoPi * fc / (float) sr);
            ch[ci].hp_a = (1.f - k);
            ch[ci].hp_y = 0.f; ch[ci].hp_x1 = 0.f;

            // speech bed: HP ~120 Hz, LP ~12 kHz
            const float fch = 120.f, fcl = 12000.f;
            const float kh  = std::exp(-juce::MathConstants<float>::twoPi * fch / (float) sr);
            const float kl  = std::exp(-juce::MathConstants<float>::twoPi * fcl / (float) sr);
            ch[ci].sp_hp_a = (1.f - kh);
            ch[ci].sp_lp_a = (1.f - kl);
            ch[ci].sp_hp_y = ch[ci].sp_hp_x1 = ch[ci].sp_lp_y = 0.f;
        }

        dryTmp.setSize(channels, 0, false, false, true);
    }

    void reset()
    {
        for (auto& c : ch) c.reset();
        dryTmp.setSize(channels, 0, false, false, true);
    }

    void setShiftAmount(float a) noexcept
    {
        amountSm.setTargetValue(juce::jlimit(-50.f, 50.f, a));
    }

    void setMix(float m) noexcept
    {
        mixSm.setTargetValue(juce::jlimit(0.f, 1.f, m));
    }

    void setFormantFrequencies(int channel, const std::vector<float>& fHz)
    {
        if (channel < 0 || channel >= channels) return;
        if (fHz.empty()) return; // keep last good on dropouts

        std::vector<float> v;
        v.reserve(maxFormants);
        for (float f : fHz)
            if (f >= 120.f && f <= 3500.f && std::isfinite(f))
                v.push_back(f);

        std::sort(v.begin(), v.end());
        if ((int) v.size() > maxFormants) v.resize(maxFormants);
        lastGoodFreqs[(size_t) channel] = std::move(v);
    }

    void processBlock(juce::AudioBuffer<float>& buffer) noexcept
    {
        juce::ScopedNoDenormals nd;
        const int ns  = buffer.getNumSamples();
        const int chIn = buffer.getNumChannels();
        if (chIn == 0 || ns == 0) return;

        const int chs = std::min(channels, chIn);
        const float nyq = 0.5f * (float) sr;

        // copy detector snapshot
        std::array<std::array<float, maxFormants>, maxChannels> peaks{};
        std::array<int, maxChannels> nPeaks{};
        for (int ci = 0; ci < chs; ++ci)
        {
            const auto& src = lastGoodFreqs[(size_t) ci];
            const int n = (int) std::min<size_t>(src.size(), maxFormants);
            nPeaks[(size_t) ci] = juce::jlimit(0, maxFormants, n);
            for (int i = 0; i < nPeaks[(size_t) ci]; ++i)
            {
                const float f = clampHz(src[(size_t) i], nyq);
                peaks[(size_t) ci][(size_t) i] = std::isfinite(f) ? f : 1000.f;
            }
        }

        // dry copy if needed (reused buffer)
        const float mTarget = mixSm.getTargetValue();
        if (mTarget < 0.999f)
        {
            if (dryTmp.getNumChannels() != chs || dryTmp.getNumSamples() != ns)
                dryTmp.setSize(chs, ns, false, false, true);
            for (int ci = 0; ci < chs; ++ci)
                dryTmp.copyFrom(ci, 0, buffer, ci, 0, ns);
        }

        // keep boosts sensible
        const float amtAbs = std::abs(amountSm.getTargetValue());
        const float maxMidBoostDb = juce::jmap(amtAbs, 0.f, 50.f, 1.5f, 4.f);

        for (int ci = 0; ci < chs; ++ci)
        {
            auto& C = ch[(size_t) ci];
            float* out = buffer.getWritePointer(ci);

            C.nActive = juce::jlimit(0, maxFormants, nPeaks[(size_t) ci]);
            float hp_y = C.hp_y, hp_x1 = C.hp_x1, hp_a = C.hp_a;
            float sp_hp_y = C.sp_hp_y, sp_hp_x1 = C.sp_hp_x1, sp_hp_a = C.sp_hp_a;
            float sp_lp_y = C.sp_lp_y, sp_lp_a = C.sp_lp_a;

            const int chunk = 32;
            for (int base = 0; base < ns; base += chunk)
            {
                const int nThis = std::min(chunk, ns - base);
                const float aNow  = amountSm.getNextValue();
                const float ratio = amountToRatio(aNow);

                float accLin = 0.f;
                for (int i = 0; i < C.nActive; ++i)
                {
                    const float f0 = peaks[(size_t) ci][(size_t) i];
                    const float f1 = clampHz(f0 * ratio, nyq);
                    const float q1 = pickQ(f1);

                    float cap = maxMidBoostDb;
                    if (f1 < 400.f)       cap *= juce::jmap(f1, 80.f, 400.f, 0.35f, 1.f);
                    else if (f1 > 3000.f) cap *= juce::jmap(f1, 3000.f, nyq - 200.f, 1.f, 0.85f);

                    C.bands[(size_t) i].setTargets(f1, q1, cap);
                    accLin += dbToLin(cap);
                }

                const float avgBoostLin = (C.nActive > 0 ? accLin / (float) C.nActive : 1.f);
                const float trim = 1.f / juce::jlimit(1.f, 2.5f, avgBoostLin);
                C.outTrim.setTargetValue(trim);

                for (int n = 0; n < nThis; ++n)
                {
                    const int idx = base + n;
                    const float x = out[idx];

                    float y = 0.f;
                    for (int i = 0; i < C.nActive; ++i)
                        y += C.bands[(size_t) i].process(x);

                    // tiny high-passed "air"
                    const float hp = x - hp_x1;
                    hp_y += hp_a * (hp - hp_y);
                    hp_x1 = x;

                    float wet = (y + 0.15f * hp_y) * C.outTrim.getNextValue();

                    // gentle broadband speech bed (HP 120, LP 12k), mixed low
                    const float d_hp = x - sp_hp_x1;
                    sp_hp_y += sp_hp_a * (d_hp - sp_hp_y);
                    sp_hp_x1 = x;
                    sp_lp_y += sp_lp_a * (sp_hp_y - sp_lp_y);
                    wet += 0.1f * sp_lp_y;

                    if (!std::isfinite(wet))
                    {
                        // hard-reset bands if anything blew up
                        for (int i = 0; i < C.nActive; ++i) C.bands[(size_t) i].reset();
                        hp_y = 0.f; sp_hp_y = 0.f; sp_lp_y = 0.f;
                        wet = 0.f;
                    }

                    out[idx] = wet;
                }
            }

            // store filter states
            C.hp_y = tinyDenormGuard(hp_y);
            C.hp_x1 = hp_x1;
            C.sp_hp_y = tinyDenormGuard(sp_hp_y);
            C.sp_hp_x1 = sp_hp_x1;
            C.sp_lp_y = tinyDenormGuard(sp_lp_y);
        }

        // final dry/wet
        const float m = mixSm.getNextValue();
        if (m < 0.999f)
        {
            for (int ci = 0; ci < chs; ++ci)
            {
                float* w = buffer.getWritePointer(ci);
                const float* d = dryTmp.getReadPointer(ci);
                for (int n = 0; n < ns; ++n)
                {
                    const float s = (1.f - m) * d[n] + m * w[n];
                    w[n] = std::isfinite(s) ? s : 0.f;
                }
            }
        }
    }
};

// ===== out-of-line ctor/dtor (after Impl is complete) =====
FormantShifter::FormantShifter() : impl(std::make_unique<Impl>()) {}
FormantShifter::~FormantShifter() noexcept = default;

// ===== thin wrappers =====
void FormantShifter::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(maxBlockSize);
    impl->prepare(sampleRate, maxBlockSize, numChannels);
}

void FormantShifter::reset()
{
    impl->reset();
}

void FormantShifter::setShiftAmount(float a) noexcept
{
    impl->setShiftAmount(a);
}

void FormantShifter::setMix(float m) noexcept
{
    impl->setMix(m);
}

void FormantShifter::setFormantFrequencies(int ch, const std::vector<float>& freqsHz)
{
    impl->setFormantFrequencies(ch, freqsHz);
}

void FormantShifter::processBlock(juce::AudioBuffer<float>& buffer) noexcept
{
    impl->processBlock(buffer);
}