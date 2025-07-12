#import <Cocoa/Cocoa.h>

@interface Computer : NSObject
@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) NSString *status;
@property (nonatomic, assign) NSInteger computerId;
@end

@implementation Computer
- (instancetype)initWithId:(NSInteger)computerId name:(NSString *)name status:(NSString *)status {
  self = [super init];
  if (self) {
    _computerId = computerId;
    _name = name;
    _status = status;
  }
  return self;
}
@end

@interface DropZone : NSView
@property (nonatomic, assign) NSInteger zoneId;
@property (nonatomic, strong) Computer *assignedComputer;
@property (nonatomic, strong) NSString *zoneName;
@end

@implementation DropZone

- (instancetype)initWithFrame:(NSRect)frame zoneId:(NSInteger)zoneId {
  self = [super initWithFrame:frame];
  if (self) {
    _zoneId = zoneId;
    _zoneName = [NSString stringWithFormat:@"Zone %ld", (long)zoneId];
    [self registerForDraggedTypes:@[NSPasteboardTypeString]];
    self.wantsLayer = YES;
    self.layer.cornerRadius = 8.0;
    self.layer.borderWidth = 2.0;
    self.layer.borderColor = [NSColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0].CGColor;
    self.layer.backgroundColor = [NSColor colorWithRed:0.95 green:0.9 blue:0.85 alpha:1.0].CGColor;
  }
  return self;
}

- (void)drawRect:(NSRect)dirtyRect {
  [super drawRect:dirtyRect];
  
  // Draw zone label
  NSString *displayText = _assignedComputer ? 
    [NSString stringWithFormat:@"%@\n%@", _zoneName, _assignedComputer.name] : 
    [NSString stringWithFormat:@"%@\nDrag and drop computer here", _zoneName];
  
  NSDictionary *attributes = @{
    NSFontAttributeName: [NSFont systemFontOfSize:12],
    NSForegroundColorAttributeName: [NSColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0]
  };
  
  NSSize textSize = [displayText sizeWithAttributes:attributes];
  NSPoint textPoint = NSMakePoint(
    (self.bounds.size.width - textSize.width) / 2,
    (self.bounds.size.height - textSize.height) / 2
  );
  
  [displayText drawAtPoint:textPoint withAttributes:attributes];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  self.layer.backgroundColor = [NSColor colorWithRed:0.9 green:0.95 blue:1.0 alpha:1.0].CGColor;
  self.layer.borderColor = [NSColor colorWithRed:0.3 green:0.6 blue:1.0 alpha:1.0].CGColor;
  return NSDragOperationMove;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender {
  self.layer.backgroundColor = [NSColor colorWithRed:0.95 green:0.9 blue:0.85 alpha:1.0].CGColor;
  self.layer.borderColor = [NSColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0].CGColor;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  NSPasteboard *pasteboard = [sender draggingPasteboard];
  NSString *computerIdString = [pasteboard stringForType:NSPasteboardTypeString];
  
  if (computerIdString) {
    NSInteger computerId = [computerIdString integerValue];
    
    // Notify the main controller about the drop
    [[NSNotificationCenter defaultCenter] postNotificationName:@"ComputerDropped" 
                                                        object:self 
                                                      userInfo:@{@"computerId": @(computerId), 
                                                               @"zoneId": @(_zoneId)}];
    
    self.layer.backgroundColor = [NSColor colorWithRed:0.9 green:1.0 blue:0.9 alpha:1.0].CGColor;
    self.layer.borderColor = [NSColor colorWithRed:0.3 green:0.8 blue:0.3 alpha:1.0].CGColor;
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
      self.layer.backgroundColor = [NSColor colorWithRed:0.95 green:0.9 blue:0.85 alpha:1.0].CGColor;
      self.layer.borderColor = [NSColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0].CGColor;
      [self setNeedsDisplay:YES];
    });
    
    return YES;
  }
  return NO;
}

@end

@interface ComputerCell : NSTableCellView
@property (nonatomic, strong) Computer *computer;
@end

@implementation ComputerCell

- (void)setComputer:(Computer *)computer {
  _computer = computer;
  self.textField.stringValue = computer.name;
  self.detailTextField.stringValue = computer.status;
}

- (void)mouseDown:(NSEvent *)event {
  [super mouseDown:event];
  
  // Start drag operation
  NSPasteboard *pasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
  [pasteboard declareTypes:@[NSPasteboardTypeString] owner:self];
  [pasteboard setString:[@(_computer.computerId) stringValue] forType:NSPasteboardTypeString];
  
  NSImage *dragImage = [[NSImage alloc] initWithSize:NSMakeSize(100, 30)];
  [dragImage lockFocus];
  [[NSColor colorWithRed:0.2 green:0.4 blue:0.8 alpha:0.8] setFill];
  NSRectFill(NSMakeRect(0, 0, 100, 30));
  [_computer.name drawAtPoint:NSMakePoint(5, 8) withAttributes:@{
    NSFontAttributeName: [NSFont systemFontOfSize:12],
    NSForegroundColorAttributeName: [NSColor whiteColor]
  }];
  [dragImage unlockFocus];
  
  [self dragImage:dragImage
               at:NSMakePoint(0, 0)
           offset:NSZeroSize
            event:event
       pasteboard:pasteboard
           source:self
        slideBack:YES];
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  return NSDragOperationMove;
}

@end

@interface MainViewController : NSViewController <NSTableViewDataSource, NSTableViewDelegate>
@property (nonatomic, strong) NSArray<Computer *> *computers;
@property (nonatomic, strong) NSArray<DropZone *> *dropZones;
@property (nonatomic, strong) NSTableView *computerTableView;
@property (nonatomic, strong) NSView *dropZoneContainer;
@end

@implementation MainViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  [self setupData];
  [self setupUI];
  [self setupNotifications];
}

- (void)setupData {
  NSMutableArray *computers = [NSMutableArray array];
  for (int i = 1; i <= 10; i++) {
    Computer *computer = [[Computer alloc] initWithId:i 
                                                 name:[NSString stringWithFormat:@"Computer %d", i]
                                               status:@"Available"];
    [computers addObject:computer];
  }
  self.computers = computers;
}

- (void)setupUI {
  self.view.wantsLayer = YES;
  self.view.layer.backgroundColor = [NSColor colorWithRed:0.98 green:0.98 blue:0.98 alpha:1.0].CGColor;
  
  // Create computer list
  [self setupComputerList];
  
  // Create drop zones
  [self setupDropZones];
}

- (void)setupComputerList {
  NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(20, 20, 300, 500)];
  scrollView.hasVerticalScroller = YES;
  scrollView.borderType = NSBezelBorder;
  
  self.computerTableView = [[NSTableView alloc] initWithFrame:scrollView.bounds];
  self.computerTableView.dataSource = self;
  self.computerTableView.delegate = self;
  self.computerTableView.headerView = nil;
  self.computerTableView.rowHeight = 50;
  
  NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:@"computer"];
  column.width = 280;
  [self.computerTableView addTableColumn:column];
  
  scrollView.documentView = self.computerTableView;
  [self.view addSubview:scrollView];
  
  // Add title
  NSTextField *titleLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 530, 300, 30)];
  titleLabel.stringValue = @"Computers";
  titleLabel.font = [NSFont boldSystemFontOfSize:18];
  titleLabel.textColor = [NSColor blackColor];
  titleLabel.backgroundColor = [NSColor clearColor];
  titleLabel.bordered = NO;
  titleLabel.editable = NO;
  [self.view addSubview:titleLabel];
}

- (void)setupDropZones {
  NSTextField *titleLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(350, 530, 300, 30)];
  titleLabel.stringValue = @"Drop Zones";
  titleLabel.font = [NSFont boldSystemFontOfSize:18];
  titleLabel.textColor = [NSColor blackColor];
  titleLabel.backgroundColor = [NSColor clearColor];
  titleLabel.bordered = NO;
  titleLabel.editable = NO;
  [self.view addSubview:titleLabel];
  
  NSMutableArray *zones = [NSMutableArray array];
  
  // Create 2 rows of 5 zones each
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 5; col++) {
      int zoneId = (row * 5) + col + 1;
      CGFloat x = 350 + (col * 150);
      CGFloat y = 300 - (row * 150);
      
      DropZone *zone = [[DropZone alloc] initWithFrame:NSMakeRect(x, y, 140, 120) zoneId:zoneId];
      [zones addObject:zone];
      [self.view addSubview:zone];
    }
  }
  
  self.dropZones = zones;
}

- (void)setupNotifications {
  [[NSNotificationCenter defaultCenter] addObserver:self 
                                           selector:@selector(handleComputerDropped:) 
                                               name:@"ComputerDropped" 
                                             object:nil];
}

- (void)handleComputerDropped:(NSNotification *)notification {
  NSInteger computerId = [notification.userInfo[@"computerId"] integerValue];
  NSInteger zoneId = [notification.userInfo[@"zoneId"] integerValue];
  
  // Find the computer
  Computer *computer = nil;
  for (Computer *comp in self.computers) {
    if (comp.computerId == computerId) {
      computer = comp;
      break;
    }
  }
  
  // Find the zone
  DropZone *zone = nil;
  for (DropZone *dropZone in self.dropZones) {
    if (dropZone.zoneId == zoneId) {
      zone = dropZone;
      break;
    }
  }
  
  if (computer && zone) {
    // Remove computer from any existing zone
    for (DropZone *existingZone in self.dropZones) {
      if (existingZone.assignedComputer == computer) {
        existingZone.assignedComputer = nil;
        [existingZone setNeedsDisplay:YES];
      }
    }
    
    // Assign to new zone
    zone.assignedComputer = computer;
    [zone setNeedsDisplay:YES];
    
    NSLog(@"Computer %@ assigned to Zone %ld", computer.name, (long)zoneId);
  }
}

#pragma mark - NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
  return self.computers.count;
}

#pragma mark - NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
  Computer *computer = self.computers[row];
  
  ComputerCell *cell = [tableView makeViewWithIdentifier:@"ComputerCell" owner:self];
  if (!cell) {
    cell = [[ComputerCell alloc] initWithFrame:NSMakeRect(0, 0, 280, 50)];
    cell.identifier = @"ComputerCell";
    
    // Add computer icon
    NSImageView *iconView = [[NSImageView alloc] initWithFrame:NSMakeRect(10, 10, 30, 30)];
    iconView.image = [NSImage imageNamed:NSImageNameComputer];
    [cell addSubview:iconView];
    
    // Add computer name label
    NSTextField *nameLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(50, 20, 200, 20)];
    nameLabel.font = [NSFont systemFontOfSize:14];
    nameLabel.textColor = [NSColor blackColor];
    nameLabel.backgroundColor = [NSColor clearColor];
    nameLabel.bordered = NO;
    nameLabel.editable = NO;
    cell.textField = nameLabel;
    [cell addSubview:nameLabel];
    
    // Add status label
    NSTextField *statusLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(50, 5, 200, 15)];
    statusLabel.font = [NSFont systemFontOfSize:12];
    statusLabel.textColor = [NSColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
    statusLabel.backgroundColor = [NSColor clearColor];
    statusLabel.bordered = NO;
    statusLabel.editable = NO;
    cell.detailTextField = statusLabel;
    [cell addSubview:statusLabel];
    
    // Add chevron
    NSTextField *chevronLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(250, 15, 20, 20)];
    chevronLabel.stringValue = @">";
    chevronLabel.font = [NSFont systemFontOfSize:16];
    chevronLabel.textColor = [NSColor colorWithRed:0.7 green:0.7 blue:0.7 alpha:1.0];
    chevronLabel.backgroundColor = [NSColor clearColor];
    chevronLabel.bordered = NO;
    chevronLabel.editable = NO;
    chevronLabel.alignment = NSTextAlignmentCenter;
    [cell addSubview:chevronLabel];
  }
  
  cell.computer = computer;
  return cell;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@endColor];
        nameLabel.backgroundColor = [NSColor clearColor];
        nameLabel.bordered = NO;
        nameLabel.editable = NO;
        cell.textField = nameLabel;
        [cell addSubview:nameLabel];
        
        // Add status label
        NSTextField *statusLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(50, 5, 200, 15)];
        statusLabel.font = [NSFont systemFontOfSize:12];
        statusLabel.textColor = [NSColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
        statusLabel.backgroundColor = [NSColor clearColor];
        statusLabel.bordered = NO;
        statusLabel.editable = NO;
        cell.detailTextField = statusLabel;
        [cell addSubview:statusLabel];
        
        // Add chevron
        NSTextField *chevronLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(250, 15, 20, 20)];
        chevronLabel.stringValue = @">";
        chevronLabel.font = [NSFont systemFontOfSize:16];
        chevronLabel.textColor = [NSColor colorWithRed:0.7 green:0.7 blue:0.7 alpha:1.0];
        chevronLabel.backgroundColor = [NSColor clearColor];
        chevronLabel.bordered = NO;
        chevronLabel.editable = NO;
        chevronLabel.alignment = NSTextAlignmentCenter;
        [cell addSubview:chevronLabel];
    }
    
    cell.computer = computer;
    return cell;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) MainViewController *mainViewController;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1100, 600)
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    
    self.window.title = @"Computer Management";
    [self.window center];
    
    self.mainViewController = [[MainViewController alloc] init];
    self.window.contentViewController = self.mainViewController;
    
    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end

// Main function
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        app.delegate = delegate;
        [app run];
    }
    return 0;
}