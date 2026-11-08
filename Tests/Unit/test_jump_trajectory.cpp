// =============================================================================
//  Tests/Unit/test_jump_trajectory.cpp
//
//  Verificação analítica do Commitment Jump a 60° (Aramini 2018 / Swink 2008).
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/JumpTrajectory.h"
#include "Core/Config.h"

#include <cmath>

TEST_SUITE("JumpTrajectory — Analise Balistica Analitica") {

    TEST_CASE("Velocidade inicial de salto nos extremos de carga") {
        CHECK(logic::JumpTrajectory::initialSpeed(0.0f) == doctest::Approx(config::PLAYER_MIN_JUMP));
        CHECK(logic::JumpTrajectory::initialSpeed(1.0f) == doctest::Approx(config::PLAYER_MAX_JUMP));
        CHECK(logic::JumpTrajectory::initialSpeed(0.5f) ==
              doctest::Approx((config::PLAYER_MIN_JUMP + config::PLAYER_MAX_JUMP) * 0.5f));

        // Valores fora do intervalo [0, 1] são clamped
        CHECK(logic::JumpTrajectory::initialSpeed(-1.0f) == doctest::Approx(config::PLAYER_MIN_JUMP));
        CHECK(logic::JumpTrajectory::initialSpeed(2.0f) == doctest::Approx(config::PLAYER_MAX_JUMP));
    }

    TEST_CASE("Angulo fixo de 60 graus e direcao de lancamento") {
        const auto vRight = logic::JumpTrajectory::initialVelocity(1.0f, 1.0f);
        CHECK(vRight.x > 0.0f);
        CHECK(vRight.y > 0.0f);
        // tan(60°) = sqrt(3) ≈ 1.73205
        const float tanTheta = vRight.y / vRight.x;
        CHECK(tanTheta == doctest::Approx(std::sqrt(3.0f)).epsilon(0.001f));

        const auto vLeft = logic::JumpTrajectory::initialVelocity(1.0f, -1.0f);
        CHECK(vLeft.x < 0.0f);
        CHECK(vLeft.y == doctest::Approx(vRight.y));
        CHECK(std::abs(vLeft.x) == doctest::Approx(vRight.x));
    }

    TEST_CASE("Calculo analitico de apice e altura maxima") {
        const auto vMax = logic::JumpTrajectory::initialVelocity(1.0f, 1.0f);
        const float tApex = logic::JumpTrajectory::timeToApex(vMax.y);
        REQUIRE(tApex > 0.0f);

        // No ápice, a velocidade vertical deve ser 0
        const auto vAtApex = logic::JumpTrajectory::velocityAt(vMax, tApex);
        CHECK(vAtApex.y == doctest::Approx(0.0f).epsilon(0.0001f));

        // Altura no ápice coincide com vy^2 / (2 * |g|)
        const float expectedHeight = (vMax.y * vMax.y) / (2.0f * std::abs(config::GRAVITY));
        CHECK(logic::JumpTrajectory::apexHeight(vMax.y) == doctest::Approx(expectedHeight));
    }

    TEST_CASE("Envelope analitico de alcance a partir da origem") {
        const logic::Vec2 origin{100.0f, 16.0f};
        const auto envelope = logic::JumpTrajectory::computeEnvelope(origin, 1.0f);

        CHECK(envelope.minApexHeight > 0.0f);
        CHECK(envelope.maxApexHeight > envelope.minApexHeight);
        CHECK(envelope.minReachX > origin.x);
        CHECK(envelope.maxReachX > envelope.minReachX);

        // Para salto à esquerda, os limites de alcance devem ser menores que origin.x
        const auto envelopeLeft = logic::JumpTrajectory::computeEnvelope(origin, -1.0f);
        CHECK(envelopeLeft.minReachX < origin.x);
        CHECK(envelopeLeft.maxReachX < origin.x);
    }

    TEST_CASE("Amostragem de trajetoria discreta e invariantes cinematicas") {
        const logic::Vec2 origin{50.0f, 16.0f};
        const auto samples = logic::JumpTrajectory::sampleTrajectory(origin, 0.75f, 1.0f, 0.5f, 0.016f);

        REQUIRE(!samples.empty());
        CHECK(samples.front().time == doctest::Approx(0.0f));
        CHECK(samples.front().position.x == doctest::Approx(origin.x));
        CHECK(samples.front().position.y == doctest::Approx(origin.y));

        // O tempo deve ser estritamente monotónico e a posição X deve avançar para a direita
        for (std::size_t i = 1; i < samples.size(); ++i) {
            CHECK(samples[i].time > samples[i - 1].time);
            CHECK(samples[i].position.x > samples[i - 1].position.x);
        }
    }
}
