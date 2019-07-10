/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.heif.utility.miaf;

import android.util.Log;

import com.nokia.heif.*;
import com.nokia.heif.Exception;
import com.nokia.heif.utility.codec.DecoderUtil;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import static com.nokia.heif.AuxiliaryProperty.ALPHA_MASK_URN;
import static com.nokia.heif.AuxiliaryProperty.DEPTH_URN;

public class MIAFReader extends HEIF
{
    private static String TAG = "MIAFReader";

    public MIAFReader() {
        super();
    }

    /**
     * Query MIAF file content.
     * @param constraints Dimensions constraints and content preference.
     * @param roles Roles of wanted images (master, thumbnail, aux, all)
     * @return Retrieved content.
     * @throws Exception
     */
    public Content getContent(Constraints constraints, Set<OutputRole> roles) throws Exception {
        ImageItem primaryItem = null;
        primaryItem = super.getPrimaryImage();
        if (primaryItem == null) {
            throw new Exception(ErrorHandler.PRIMARY_ITEM_NOT_SET, "No mandatory primary item in MIAF file.");
        }

        Base seed = null;

        // Is there an alternate group which includes the primary item?
        List<Base> alternatives = getAlternateGroupMembers(primaryItem);
        if (alternatives == null) { // no alternative group
            // If no track preference is indicated, return the primary item.
            if (constraints.preference != Preference.TRACK) {
                seed = primaryItem;
            }
            else { // otherwise seed is set to equal to any track that fills requirements
                List<Track> tracks = super.getTracks();
                for (Track track : tracks) {
                    // TODO should also check that the track conforms to MIAF specification
                    if (checkConstraints(track, constraints) == true) {
                        seed = track;
                        break;
                    }
                }
            }
        } else { // There is an alternative group.
            // Is track preference indicated?
            if (constraints.preference == Preference.TRACK) {
                for (Base entry : alternatives) {
                    if (entry instanceof Track &&
                            checkConstraints(entry, constraints)) {
                        seed = entry;
                        break;
                    }
                }
            } else if (constraints.preference == Preference.IMAGE) {
                for (Base entry : alternatives) {
                    if (entry instanceof ImageItem &&
                            checkConstraints(entry, constraints)) {
                        seed = entry;
                        break;
                    }
                }
            } else { // no preference
                for (Base entry : alternatives) {
                    if (checkConstraints(entry, constraints)) {
                        seed = entry;
                        break;
                    }
                }
            }
        }

        if (roles.contains(OutputRole.ALL)) {
            roles.add(OutputRole.MASTER);
            roles.add(OutputRole.THUMBNAIL);
            roles.add(OutputRole.AUXILIARY_ALL);
        }
        if (roles.contains(OutputRole.AUXILIARY_ALL)) {
            roles.add(OutputRole.AUXILIARY_ALPHA);
            roles.add(OutputRole.AUXILIARY_DEPTH);
        }

        Content content = null;
        if (seed instanceof Track) {
            content = filterRolesTrack((Track) seed, roles);
        } else if (seed instanceof ImageItem) {
            content = filterRolesItem((ImageItem) seed, roles);
        }
        // TODO
        //  -Error if track or item preference is set, but just the other content is available?
        //  -One or more roles are provided as input and selectedIds contains a track_ID or
        //   item_ID value of a track or item that has a role other than those provided as input.

        return content;
    }

    // Helper methods

    /**
     * Return alternative group members.
     * @param object Track or item to search from alternate groups.
     * @return List of alternate group members, null if the object was not found from any alternative gruop.
     * @throws Exception
     */
    private List<Base> getAlternateGroupMembers(Base object) throws Exception {
        List<EntityGroup> groups = super.getEntityGroupsByType(FOURCC_ALTERNATE);
        for (EntityGroup group : groups) {
            List<Base> members = group.getMembers();
            if (members.contains(object))
            {
                return members;
            }
        }

        return null;
    }

    /**
     * @param input An VideoTrack or a VideoTrack.
     * @param constraints Constraints for the wanted content: maximum dimensions.
     * @return True if the input fulfils constraints and is decodable.
     * @throws Exception in case input is not a video track or an image item.
     */
    private Boolean checkConstraints(Base input, Constraints constraints) throws Exception {
        Size size;
        if (input instanceof VideoTrack) {
            VideoTrack track = (VideoTrack) input;
            size = track.getDisplaySize();
        } else if (input instanceof ImageItem) {
            ImageItem imageItem = (ImageItem) input;
            size = imageItem.getSize();
        } else {
            throw new Exception(ErrorHandler.UNDEFINED_ERROR, "Base not track or image item.");
        }

        if (constraints.maxHeight > 0 &&
                size.height > constraints.maxHeight) {
            return false;
        }

        if (constraints.maxWidth > 0 &&
                size.width > constraints.maxWidth) {
            return false;
        }

        return isDecodable(input);
    }

    /**
     * Check if it is possible to decode an image item or a video track (/image sequence).
     * @param base An image item or a track.
     * @return True if it is possible to decode the item, false otherwise.
     */
    private Boolean isDecodable(Base base)
    {
        if (base instanceof ImageItem)
        {
            try
            {
                if (base instanceof CodedImageItem)
                {
                    return DecoderUtil.isDecodable(base);
                }
                if (base instanceof GridImageItem)
                {
                    GridImageItem gridItem = (GridImageItem) base;

                    // In a MIAF file all input images of a grid use same decoder configuration,
                    // so it is enough to test the first one.
                    if (gridItem.getColumnCount() == 0 || gridItem.getRowCount() == 0)
                    {
                        return true; // empty grid
                    }
                    ImageItem image = gridItem.getImage(0, 0);
                    return isDecodable(image);
                }
                if (base instanceof OverlayImageItem)
                {
                    // Check all items in the overlay.
                    List<OverlayImageItem.OverlayedImage> overlayItems = ((OverlayImageItem) base).getOverlayedImages();
                    for (OverlayImageItem.OverlayedImage overlayImage : overlayItems)
                    {
                        if (isDecodable(overlayImage.image) == false)
                        {
                            return false;
                        }
                    }
                    return true;
                }
                if (base instanceof IdentityImageItem)
                {
                    return isDecodable(((IdentityImageItem) base).getImage());
                }
            }
            catch (java.lang.Exception e)
            {
                Log.e(TAG, "isDecodable() failed: " + e.getMessage());
                return false;
            }
        }
        else if (base instanceof VideoTrack)
        {
            try
            {
                VideoTrack track = (VideoTrack)base;
                List<VideoSample> samples = track.getVideoSamples();
                if (samples.size() == 0) return false;
                return DecoderUtil.isDecodable(samples.get(0));
            }
            catch (java.lang.Exception e)
            {
                Log.e(TAG, "isDecodable() failed: " + e.getMessage());
                return false;
            }
        }

        return false;
    }

    private Content filterRolesTrack(Track track, Set<OutputRole> roles) throws Exception {
        Content content = new Content();

        if (roles.contains(OutputRole.MASTER)) {
            content.master = track;
        }

        if (roles.contains(OutputRole.THUMBNAIL)) {
            content.thumbTracks = track.getThumbnails();
        }

        List<Track> auxTracks = track.getAuxiliaries();
        for (Track auxTrack : auxTracks)
        {
            String aux = ((ImageSequence) auxTrack).getAuxTypeInfo();
            if ((roles.contains(OutputRole.AUXILIARY_ALPHA) && aux.equals(ALPHA_MASK_URN)) ||
                (roles.contains(OutputRole.AUXILIARY_DEPTH) && aux.equals(DEPTH_URN)))
            {
                if (isDecodable(auxTrack)) {
                    content.auxTracks.add(auxTrack);
                    continue;
                }
                else {
                    List<Base> alternatives = getAlternateGroupMembers(auxTrack);
                    if (alternatives != null)
                    {
                        for (Base alternative : alternatives)
                        {
                            if (alternative instanceof VideoTrack &&
                                    isDecodable(alternative)) {
                                content.auxTracks.add((Track)alternative);
                            }
                        }
                    }
                }
            }
        }

        return content;
    }

    private Content filterRolesItem(ImageItem seed, Set<OutputRole> roles) throws Exception {
        Content content = new Content();

        if (roles.contains(OutputRole.MASTER)) {
            content.master = seed;
        }

        if (roles.contains(OutputRole.THUMBNAIL)) {
            content.thumbnails = seed.getThumbnails();
        }

        if (roles.contains(OutputRole.AUXILIARY_ALPHA) ||
                roles.contains(OutputRole.AUXILIARY_DEPTH)) {
            List<ImageItem> auxs = seed.getAuxiliaries();
            for (ImageItem aux : auxs) {
                final AuxiliaryProperty prop = aux.getAuxiliaryProperty();
                final String propertyType = prop.getType();
                if ((roles.contains(OutputRole.AUXILIARY_ALPHA) && propertyType.equals(ALPHA_MASK_URN)) ||
                        (roles.contains(OutputRole.AUXILIARY_DEPTH) && propertyType.equals(AuxiliaryProperty.DEPTH_URN)))
                {
                    content.auxiliaries.add(aux);
                }
            }
        }

        return content;
    }

    /**
     * Image and track roles for MIAF content queries
     */
    public enum OutputRole {
        ALL,
        MASTER,
        THUMBNAIL,
        AUXILIARY_ALL,
        AUXILIARY_ALPHA,
        AUXILIARY_DEPTH
    }

    public enum Preference {
        IMAGE,
        TRACK,
        NO_PREFERENCE
    }

    /**
     * Constraints for MIAF content queries.
     */
    public static class Constraints {
        public int maxWidth;
        public int maxHeight;
        public Preference preference;
    }

    public class Content {
        public Base master;
        public List<ImageItem> thumbnails = new ArrayList<>();
        public List<ImageItem> auxiliaries = new ArrayList<>();

        public List<Track> thumbTracks = new ArrayList<>();
        public List<Track> auxTracks = new ArrayList<>();
    }
}
