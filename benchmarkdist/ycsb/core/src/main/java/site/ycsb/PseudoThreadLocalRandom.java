/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * This file is available under and governed by the GNU General Public
 * License version 2 only, as published by the Free Software Foundation.
 * However, the following notice accompanied the original version of this
 * file:
 *
 * Written by Doug Lea with assistance from members of JCP JSR-166
 * Expert Group and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

package site.ycsb;

import java.util.Random;

/**
 * A random number generator isolated to the current thread. Like the
 * global {@link java.util.Random} generator used by the {@link
 * java.lang.Math} class, a {@code PseudoThreadLocalRandom} is initialized
 * with an internally generated seed that may not otherwise be
 * modified. When applicable, use of {@code PseudoThreadLocalRandom} rather
 * than shared {@code Random} objects in concurrent programs will
 * typically encounter much less overhead and contention. Use of
 * {@code PseudoThreadLocalRandom} is particularly appropriate when multiple
 * tasks (for example, each a {@link ForkJoinTask}) use random numbers
 * in parallel in thread pools.
 *
 * <p>
 * Usages of this class should typically be of the form:
 * {@code PseudoThreadLocalRandom.current().nextX(...)} (where
 * {@code X} is {@code Int}, {@code Long}, etc).
 * When all usages are of this form, it is never possible to
 * accidently share a {@code PseudoThreadLocalRandom} across multiple threads.
 *
 * <p>
 * This class also provides additional commonly used bounded random
 * generation methods.
 *
 * @since 1.7
 * @author Doug Lea
 */
public class PseudoThreadLocalRandom extends Random {
  // same constants as Random, but must be redeclared because private
  private static final long MULTIPLIER = 0x5DEECE66DL;
  private static final long ADD_END = 0xBL;
  private static final long MASK = (1L << 48) - 1;

  /**
   * The random seed. We can't use super.seed.
   */
  private long rnd;

  /**
   * Initialization flag to permit calls to setSeed to succeed only
   * while executing the Random constructor. We can't allow others
   * since it would cause setting seed in one part of a program to
   * unintentionally impact other usages by the thread.
   */
  private boolean initialized;

  // Padding to help avoid memory contention among seed updates in
  // different TLRs in the common case that they are located near
  // each other.
  private long pad0, pad1, pad2, pad3, pad4, pad5, pad6, pad7;

  /**
   * The actual ThreadLocal.
   */
  private static final ThreadLocal<PseudoThreadLocalRandom> LOCAL_RANDOM = new ThreadLocal<PseudoThreadLocalRandom>() {
    protected PseudoThreadLocalRandom initialValue() {
      return new PseudoThreadLocalRandom();
    }
  };

  /**
   * Constructor called only by LOCAL_RANDOM.initialValue.
   */
  PseudoThreadLocalRandom() {
    super();
    int threadid = (int)Thread.currentThread().getId(); // always starting from 1
    //System.out.println(String.format("threadid: %d", threadid));
    setSeed(threadid);
    initialized = true;
  }

  /**
   * Returns the current thread's {@code PseudoThreadLocalRandom}.
   *
   * @return the current thread's {@code PseudoThreadLocalRandom}
   */
  public static PseudoThreadLocalRandom current() {
    return LOCAL_RANDOM.get();
  }

  /**
   * Throws {@code UnsupportedOperationException}. Setting seeds in
   * this generator is not supported.
   *
   * @throws UnsupportedOperationException always
   */
  public void setSeed(long seed) {
    if (initialized) {
      throw new UnsupportedOperationException();
    }

    rnd = (seed ^ MULTIPLIER) & MASK;
  }

  protected int next(int bits) {
    rnd = (rnd * MULTIPLIER + ADD_END) & MASK;
    return (int) (rnd >>> (48 - bits));
  }

  /**
   * Returns a pseudorandom, uniformly distributed value between the
   * given least value (inclusive) and bound (exclusive).
   *
   * @param least the least value returned
   * @param bound the upper bound (exclusive)
   * @throws IllegalArgumentException if least greater than or equal
   *                                  to bound
   * @return the next value
   */
  public int nextInt(int least, int bound) {
    if (least >= bound) {
      throw new IllegalArgumentException();
    }
    return nextInt(bound - least) + least;
  }

  /**
   * Returns a pseudorandom, uniformly distributed value
   * between 0 (inclusive) and the specified value (exclusive).
   *
   * @param n the bound on the random number to be returned. Must be
   *          positive.
   * @return the next value
   * @throws IllegalArgumentException if n is not positive
   */
  public long nextLong(long n) {
    if (n <= 0) {
      throw new IllegalArgumentException("n must be positive");
    }

    // Divide n by two until small enough for nextInt. On each
    // iteration (at most 31 of them but usually much less),
    // randomly choose both whether to include high bit in result
    // (offset) and whether to continue with the lower vs upper
    // half (which makes a difference only if odd).
    long offset = 0;
    while (n >= Integer.MAX_VALUE) {
      int bits = next(2);
      long half = n >>> 1;
      long nextn = ((bits & 2) == 0) ? half : n - half;
      if ((bits & 1) == 0) {
        offset += n - nextn;
      }  
      n = nextn;
    }
    return offset + nextInt((int) n);
  }

  /**
   * Returns a pseudorandom, uniformly distributed value between the
   * given least value (inclusive) and bound (exclusive).
   *
   * @param least the least value returned
   * @param bound the upper bound (exclusive)
   * @return the next value
   * @throws IllegalArgumentException if least greater than or equal
   *                                  to bound
   */
  public long nextLong(long least, long bound) {
    if (least >= bound) {
      throw new IllegalArgumentException();
    }
    return nextLong(bound - least) + least;
  }

  /**
   * Returns a pseudorandom, uniformly distributed {@code double} value
   * between 0 (inclusive) and the specified value (exclusive).
   *
   * @param n the bound on the random number to be returned. Must be
   *          positive.
   * @return the next value
   * @throws IllegalArgumentException if n is not positive
   */
  public double nextDouble(double n) {
    if (n <= 0) {
      throw new IllegalArgumentException("n must be positive");
    }
    return nextDouble() * n;
  }

  /**
   * Returns a pseudorandom, uniformly distributed value between the
   * given least value (inclusive) and bound (exclusive).
   *
   * @param least the least value returned
   * @param bound the upper bound (exclusive)
   * @return the next value
   * @throws IllegalArgumentException if least greater than or equal
   *                                  to bound
   */
  public double nextDouble(double least, double bound) {
    if (least >= bound) {
      throw new IllegalArgumentException();
    }
    return nextDouble() * (bound - least) + least;
  }

  private static final long serialVersionUID = -5851777807851030925L;
}
