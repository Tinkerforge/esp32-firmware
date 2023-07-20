// From: https://github.com/FreeRTOS/coreMQTT

/*
 * coreMQTT v2.1.0
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdint.h>

static bool matchEndWildcardsSpecialCases( const char * pTopicFilter,
                                           uint16_t topicFilterLength,
                                           uint16_t filterIndex )
{
    bool matchFound = false;

    /* Check if the topic filter has 2 remaining characters and it ends in
     * "/#". This check handles the case to match filter "sport/#" with topic
     * "sport". The reason is that the '#' wildcard represents the parent and
     * any number of child levels in the topic name.*/
    if( ( topicFilterLength >= 3U ) &&
        ( filterIndex == ( topicFilterLength - 3U ) ) &&
        ( pTopicFilter[ filterIndex + 1U ] == '/' ) &&
        ( pTopicFilter[ filterIndex + 2U ] == '#' ) )

    {
        matchFound = true;
    }

    /* Check if the next character is "#" or "+" and the topic filter ends in
     * "/#" or "/+". This check handles the cases to match:
     *
     * - Topic filter "sport/+" with topic "sport/".
     * - Topic filter "sport/#" with topic "sport/".
     */
    if( ( filterIndex == ( topicFilterLength - 2U ) ) &&
        ( pTopicFilter[ filterIndex ] == '/' ) )
    {
        /* Check that the last character is a wildcard. */
        matchFound = ( pTopicFilter[ filterIndex + 1U ] == '+' ) ||
                     ( pTopicFilter[ filterIndex + 1U ] == '#' );
    }

    return matchFound;
}


static bool matchWildcards( const char * pTopicName,
                            uint16_t topicNameLength,
                            const char * pTopicFilter,
                            uint16_t topicFilterLength,
                            uint16_t * pNameIndex,
                            uint16_t * pFilterIndex,
                            bool * pMatch )
{
    bool shouldStopMatching = false;
    bool locationIsValidForWildcard;

    /* Wild card in a topic filter is only valid either at the starting position
     * or when it is preceded by a '/'.*/
    locationIsValidForWildcard = ( *pFilterIndex == 0u ) ||
                                 ( pTopicFilter[ *pFilterIndex - 1U ] == '/' );

    if( ( pTopicFilter[ *pFilterIndex ] == '+' ) && ( locationIsValidForWildcard == true ) )
    {
        bool nextLevelExistsInTopicName = false;
        bool nextLevelExistsinTopicFilter = false;

        /* Move topic name index to the end of the current level. The end of the
         * current level is identified by the last character before the next level
         * separator '/'. */
        while( *pNameIndex < topicNameLength )
        {
            /* Exit the loop if we hit the level separator. */
            if( pTopicName[ *pNameIndex ] == '/' )
            {
                nextLevelExistsInTopicName = true;
                break;
            }

            ( *pNameIndex )++;
        }

        /* Determine if the topic filter contains a child level after the current level
         * represented by the '+' wildcard. */
        if( ( *pFilterIndex < ( topicFilterLength - 1U ) ) &&
            ( pTopicFilter[ *pFilterIndex + 1U ] == '/' ) )
        {
            nextLevelExistsinTopicFilter = true;
        }

        /* If the topic name contains a child level but the topic filter ends at
         * the current level, then there does not exist a match. */
        if( ( nextLevelExistsInTopicName == true ) &&
            ( nextLevelExistsinTopicFilter == false ) )
        {
            *pMatch = false;
            shouldStopMatching = true;
        }

        /* If the topic name and topic filter have child levels, then advance the
         * filter index to the level separator in the topic filter, so that match
         * can be performed in the next level.
         * Note: The name index already points to the level separator in the topic
         * name. */
        else if( nextLevelExistsInTopicName == true )
        {
            ( *pFilterIndex )++;
        }
        else
        {
            /* If we have reached here, the the loop terminated on the
             * ( *pNameIndex < topicNameLength) condition, which means that have
             * reached past the end of the topic name, and thus, we decrement the
             * index to the last character in the topic name.*/
            ( *pNameIndex )--;
        }
    }

    /* '#' matches everything remaining in the topic name. It must be the
     * last character in a topic filter. */
    else if( ( pTopicFilter[ *pFilterIndex ] == '#' ) &&
             ( *pFilterIndex == ( topicFilterLength - 1U ) ) &&
             ( locationIsValidForWildcard == true ) )
    {
        /* Subsequent characters don't need to be checked for the
         * multi-level wildcard. */
        *pMatch = true;
        shouldStopMatching = true;
    }
    else
    {
        /* Any character mismatch other than '+' or '#' means the topic
         * name does not match the topic filter. */
        *pMatch = false;
        shouldStopMatching = true;
    }

    return shouldStopMatching;
}


bool matchTopicFilter( const char * pTopicName,
                       uint16_t topicNameLength,
                       const char * pTopicFilter,
                       uint16_t topicFilterLength )
{
    bool matchFound = false, shouldStopMatching = false;
    uint16_t nameIndex = 0, filterIndex = 0;

    while( ( nameIndex < topicNameLength ) && ( filterIndex < topicFilterLength ) )
    {
        /* Check if the character in the topic name matches the corresponding
         * character in the topic filter string. */
        if( pTopicName[ nameIndex ] == pTopicFilter[ filterIndex ] )
        {
            /* If the topic name has been consumed but the topic filter has not
             * been consumed, match for special cases when the topic filter ends
             * with wildcard character. */
            if( nameIndex == ( topicNameLength - 1U ) )
            {
                matchFound = matchEndWildcardsSpecialCases( pTopicFilter,
                                                            topicFilterLength,
                                                            filterIndex );
            }
        }
        else
        {
            /* Check for matching wildcards. */
            shouldStopMatching = matchWildcards( pTopicName,
                                                 topicNameLength,
                                                 pTopicFilter,
                                                 topicFilterLength,
                                                 &nameIndex,
                                                 &filterIndex,
                                                 &matchFound );
        }

        if( ( matchFound == true ) || ( shouldStopMatching == true ) )
        {
            break;
        }

        /* Increment indexes. */
        nameIndex++;
        filterIndex++;
    }

    if( matchFound == false )
    {
        /* If the end of both strings has been reached, they match. This represents the
         * case when the topic filter contains the '+' wildcard at a non-starting position.
         * For example, when matching either of "sport/+/player" OR "sport/hockey/+" topic
         * filters with "sport/hockey/player" topic name. */
        matchFound = ( nameIndex == topicNameLength ) &&
                     ( filterIndex == topicFilterLength );
    }

    return matchFound;
}
