/*
 * Copyright 2021 Google LLC.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// An implementation of a PRNG using a HMAC-based key derivation function.
//
// HMAC-based key derivation functions (HKDF, for short) consist of two
// important functions: extract and expand. Given an input key with sufficient
// entropy, the HKDF will extract the entropy into a more uniform, unbiased
// entropy. The HKDF can expand this entropy into many pseudorandom outputs.
// Therefore, the input key must have sufficient entropy to ensure the outputs
// are pseudorandom. The pseudorandom outputs of HKDF are deterministic for any
// fixed input key allowing replay of the pseudorandom outputs by multiple
// clients by sharing the input key. For more information about HKDFs, see [1]
// for an overview and [2] for a full description.
//
// [1] https://en.wikipedia.org/wiki/HKDF
// [2] https://tools.ietf.org/html/rfc5869

#ifndef RLWE_SINGLE_THREAD_HKDF_PRNG_H_
#define RLWE_SINGLE_THREAD_HKDF_PRNG_H_

#include "absl/strings/string_view.h"
#include "prng/hkdf_prng_util.h"
#include "prng/prng.h"
#include "statusor.h"

namespace rlwe {

class SingleThreadHkdfPrng : public SecurePrng {
 public:
  // Constructs a secure pseudorandom number generator using a HMAC-based key
  // derivation function (HKDF). The parameter in_key is the key for the HKDF.
  //
  // This class is for use only in single threaded scenarios. For thread safe
  // pseudorandom number generators, use HkdfPrng instead.
  //
  // Input keys should contain sufficient randomness (such as those generated by
  // the GenerateKey() function) to ensure the random generated strings are
  // pseudorandom. As long as the initial key contains sufficient entropy, there
  // is no bound on the number of pseudorandom bytes that can be created.
  //
  // HkdfPrng allows replaying pseudorandom outputs. For any fixed input key,
  // the pseudorandom outputs of HkdfPrng will be identical.
  //
  // For a fixed key and salt, the underlying Hkdf crunchy primitive can
  // generate 255 * 32 pseudorandom bytes. Once, these bytes have been
  // exhausted, the prng deterministically re-salts the key using a salting
  // counter, thereby constructing a new internal Hkdf that can output more
  // pseudorandom bytes.
  //
  // Fails if the key is not the expected size or on internal cryptographic
  // errors.
  static rlwe::StatusOr<std::unique_ptr<SingleThreadHkdfPrng>> Create(
      absl::string_view in_key);

  // Returns 8 bits of randomness.
  //
  // Fails on internal cryptographic errors.
  rlwe::StatusOr<Uint8> Rand8() override;

  // Returns 64 bits of randomness.
  //
  // Fails on internal cryptographic errors.
  rlwe::StatusOr<Uint64> Rand64() override;

  // Generate a valid seed for the Prng.
  //
  // Fails on internal cryptographic errors.
  static rlwe::StatusOr<std::string> GenerateSeed() {
    return internal::HkdfPrngGenerateKey();
  }

  // Output the size of the expected generated seed.
  static int SeedLength() { return internal::kHkdfKeyBytesSize; }

 private:
  explicit SingleThreadHkdfPrng(absl::string_view in_key,
                                int position_in_buffer, int salt_counter,
                                std::vector<Uint8> buffer);

  const std::string key_;
  int position_in_buffer_;
  int salt_counter_;
  std::vector<Uint8> buffer_;
};

}  // namespace rlwe

#endif  // RLWE_SINGLE_THREAD_HKDF_PRNG_H_
