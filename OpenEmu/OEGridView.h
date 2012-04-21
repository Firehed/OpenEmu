/*
 Copyright (c) 2012, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>
#import "OEGridViewCell.h"
#import "OEGridViewLayoutManager.h"
#import "OEGridViewFieldEditor.h"

@class OEGridView;
@class OEMenu;

@protocol OEGridViewDelegate <NSObject>

@optional
- (void)selectionChangedInGridView:(OEGridView *)gridView;
- (void)gridView:(OEGridView *)gridView doubleClickedCellForItemAtIndex:(NSUInteger)index;
- (NSDragOperation)gridView:(OEGridView *)gridView validateDrop:(id<NSDraggingInfo>)sender;
- (NSDragOperation)gridView:(OEGridView *)gridView draggingUpdated:(id<NSDraggingInfo>)sender;
- (BOOL)gridView:(OEGridView *)gridView acceptDrop:(id<NSDraggingInfo>)sender;

@end

#pragma mark -
@protocol OEGridViewDataSource <NSObject>

@required
- (OEGridViewCell *)gridView:(OEGridView *)gridView cellForItemAtIndex:(NSUInteger)index;
- (NSUInteger)numberOfItemsInGridView:(OEGridView *)gridView;

@optional
- (NSView *)viewForNoItemsInGridView:(OEGridView *)gridView;
- (void)gridView:(OEGridView *)gridView willBeginEditingCellForItemAtIndex:(NSUInteger)index;
- (void)gridView:(OEGridView *)gridView didEndEditingCellForItemAtIndex:(NSUInteger)index;
- (id<NSPasteboardWriting>)gridView:(OEGridView *)gridView pasteboardWriterForIndex:(NSInteger)index;
- (OEMenu *)gridView:(OEGridView *)gridView menuForItemsAtIndexes:(NSIndexSet *)indexes;

@end

#pragma mark -
@interface OEGridView : NSView <OEGridViewLayoutManagerProtocol, NSDraggingSource, NSDraggingDestination>
{
@private
    OEGridLayer *_rootLayer;                        // Root layer, where all other layers are inserted into
    CALayer     *_selectionLayer;                   // Selection box that appears when selecting multiple cells
    CALayer     *_dragIndicationLayer;              // A visual indication that a file is being dragged onto the grid view
    CALayer     *_backgroundLayer;                  // A decorative background layer, the layer should return nil for -hitTest
    CALayer     *_foregroundLayer;                  // A decorative foreground layer, the layer should return nil for -hitTest
    NSView      *_noItemsView;                      // A decorative view when there are no items to show, e.g. blank slate

    NSScrollElasticity _previousElasticity;         // Caches the original elasticity of the scroller eview before the blank slate is added

    NSSize  _itemSize;                              // User defined cell size (defaults to 250 x 250)
    CGFloat _minimumColumnSpacing;                  // Minimum spacing between columns
    CGFloat _rowSpacing;                            // Minimum spacing between rows

    NSMutableIndexSet *_originalSelectionIndexes;   // Original set of indexes selected before an inverted (cmd key) selection operation
    NSMutableIndexSet *_selectionIndexes;           // Index or indexes that are currently selected
    NSUInteger _indexOfKeyboardSelection;           // Last index of the selected cell using the keyboard

    NSMutableSet      *_visibleCells;               // Cached visible cells
    NSMutableIndexSet *_visibleCellsIndexes;        // Cached indexes of the visible cells
    NSMutableSet      *_reuseableCells;             // Cached cells that are no longer in view

    NSDraggingSession *_draggingSession;            // Drag session used during a drag operation
    OEGridLayer       *_prevDragDestinationLayer;   // Previous destination cell of a drag operation, used to prevent multiple messages to same cell
    OEGridLayer       *_dragDestinationLayer;       // Destination cell of a drag operation
    NSDragOperation    _lastDragOperation;          // Last drag operation generated by -draggingEntered:
    OEGridLayer       *_trackingLayer;              // The layer receiving all the drag operations (can be root layer)
    NSPoint            _initialPoint;               // Initial position of the mouse of a drag operation

    BOOL _needsReloadData;                          // Determines if the data should be reloaded
    BOOL _needsLayoutGridView;                      // Determines if the cells should really be laid out

    NSUInteger _cachedNumberOfVisibleColumns;       // Cached number of visible columns
    NSUInteger _cachedNumberOfVisibleRows;          // Cached number of visiabl rows (include partially visible rows)
    NSUInteger _cachedNumberOfItems;                // Cached number of items in the data source

    NSPoint _cachedContentOffset;                   // Last known content offset
    NSSize  _cachedViewSize;                        // Last known view size
    NSSize  _cachedItemSize;                        // Cached cell size that includes row spacing and cached column spacing
    CGFloat _cachedColumnSpacing;                   // Cached column spacing is the dynamic spacing between columns, no less than minimumColumnSpacing

    OEGridViewFieldEditor *_fieldEditor;            // Text field editor of a CATextLayer

    struct
    {
        unsigned int selectionChanged : 1;
        unsigned int doubleClickedCellForItemAtIndex : 1;
        unsigned int validateDrop : 1;
        unsigned int draggingUpdated : 1;
        unsigned int acceptDrop : 1;
    } _delegateHas;                                 // Cached methods that the delegate implements

    struct
    {
        unsigned int viewForNoItemsInGridView : 1;
        unsigned int willBeginEditingCellForItemAtIndex : 1;
        unsigned int didEndEditingCellForItemAtIndex : 1;
        unsigned int pasteboardWriterForIndex : 1;
        unsigned int menuForItemsAtIndexes : 1;
    } _dataSourceHas;                               // Cached methods that the dataSource implements
}

#pragma mark -
#pragma mark Query Data Sources

- (id)dequeueReusableCell;
- (NSUInteger)numberOfItems;
- (OEGridViewCell *)cellForItemAtIndex:(NSUInteger)index makeIfNecessary:(BOOL)necessary;

#pragma mark -
#pragma mark Query Cells

- (NSUInteger)indexForCell:(OEGridViewCell *)cell;
- (NSUInteger)indexForCellAtPoint:(NSPoint)point;
- (NSIndexSet *)indexesForCellsInRect:(NSRect)rect;
- (NSArray *)visibleCells;
- (NSIndexSet *)indexesForVisibleCells;
- (NSRect)rectForCellAtIndex:(NSUInteger)index;

#pragma mark -
#pragma mark Selection

- (NSUInteger)indexForSelectedCell;
- (NSIndexSet *)indexesForSelectedCells;
- (void)selectCellAtIndex:(NSUInteger)index;
- (void)deselectCellAtIndex:(NSUInteger)index;
- (void)selectAll:(id)sender;
- (void)deselectAll:(id)sender;

#pragma mark -
#pragma mark Data Reload

- (void)reloadData;
- (void)reloadCellsAtIndexes:(NSIndexSet *)indexes;

#pragma mark -
#pragma mark Properties

@property(nonatomic, retain) CALayer *foregroundLayer;
@property(nonatomic, retain) CALayer *backgroundLayer;
@property(nonatomic, assign) CGFloat minimumColumnSpacing;
@property(nonatomic, assign) CGFloat rowSpacing;
@property(nonatomic, assign) CGSize itemSize;
@property(nonatomic, assign) id<OEGridViewDataSource> dataSource;
@property(nonatomic, assign) id<OEGridViewDelegate> delegate;
@property(nonatomic, copy)   NSIndexSet *selectionIndexes;

@end
