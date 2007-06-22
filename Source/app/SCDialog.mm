//
//  SCDialogs.m
//  SC3lang
//
//  Created by cruxxial on Tue Dec 17 2002.
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#import "SCDialog.h"


@implementation SCDialog

+(id)receiver:(PyrObject*)argReceiver result:(PyrObject*)argResult
{
    return [[super alloc] initWithReceiver: argReceiver result: argResult];
}

-(id)initWithReceiver:(PyrObject*)argReceiver result:(PyrObject*)argResult
{	
    if(self == [super init]) {
        receiver = argReceiver;
        result = argResult;
		openPanel = 0;
	}
    return self;
}

-(void)scvmDeferWithSelector:(SEL)selector
{
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:
                                        [self methodSignatureForSelector: selector]];
    [invocation setTarget:self];
    [invocation setSelector: selector];
    [invocation retainArguments];
    
    [[SCVirtualMachine sharedInstance] defer: invocation];
}


-(void)getPaths
{  
    if (!openPanel) {
		openPanel = [NSOpenPanel openPanel];
		[openPanel retain];
	}
	[openPanel setAllowsMultipleSelection: YES];
    if(NSOKButton == [openPanel runModalForTypes: nil]) {
         [self returnPaths: [openPanel URLs]];
    } else {
        [self cancel];
    }
}

        
-(void)returnPaths:(NSArray*)urls
{
    int i;
    int count = [urls count];
    
    VMGlobals *g = gMainVMGlobals;
    pthread_mutex_lock (&gLangMutex);
    for (i = 0; i < count; i++)
    {
        PyrString* pyrPathString = newPyrString(g->gc,[[[urls objectAtIndex: i ] path] cString],0,true);
        
        PyrSlot slot;
        SetObject(&slot, pyrPathString);
        
        result->slots[i].ucopy = slot.ucopy;

        g->gc->GCWrite(result,pyrPathString);
        // have to set size field each time in order that gc can find the created objects
        result->size = i+1;
    }
    pthread_mutex_unlock (&gLangMutex);

    [self ok];
}

-(void)savePanel
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    if([savePanel runModal] == NSFileHandlingPanelOKButton) {
        [self returnPath: [savePanel filename]];
    } else {
        [self cancel];
    }
}

-(void)returnPath:(NSString*)path
{
    // check if greater than 512
    int size = [path cStringLength];
    if(size > 512) {
        [self error];
        return;
    }
    
    pthread_mutex_lock (&gLangMutex);    
    memcpy(result->slots,[path cString], size);
    result->size = size;
    pthread_mutex_unlock (&gLangMutex);

    [self ok];
}

/*  better done using SCTextField
-(void)getStringPrompt:(NSString*)prompt defaultString:(NSString*)defaultString
{
}
- (void)loadGetStringUI {
    if (!textField) {
        if (![NSBundle loadNibNamed:@"GetStringDlg" owner:self])  {
            NSLog(@"Failed to load GetStringDlg.nib");
        }
        //if (self == sharedGetStringObject) 
        [[textField window] setFrameAutosaveName:@"GetString"];
    }
}
*/

// get color dialog

// all responses
-(void)ok
{
    pthread_mutex_lock (&gLangMutex);
		PyrSymbol *method = getsym("ok");
        VMGlobals *g = gMainVMGlobals;
        g->canCallOS = true;
        ++g->sp;  SetObject(g->sp, receiver ); 
        runInterpreter(g, method, 1);
        g->canCallOS = false;
    pthread_mutex_unlock (&gLangMutex);
}

-(void)cancel
{
    pthread_mutex_lock (&gLangMutex);
		PyrSymbol *method = getsym("cancel");
        VMGlobals *g = gMainVMGlobals;
        g->canCallOS = true;
        ++g->sp;  SetObject(g->sp, receiver ); 
        runInterpreter(g, method, 1);
        g->canCallOS = false;
    pthread_mutex_unlock (&gLangMutex);
}

-(void)error
{
    pthread_mutex_lock (&gLangMutex);
		PyrSymbol *method = getsym("errir");
        VMGlobals *g = gMainVMGlobals;
        g->canCallOS = true;
        ++g->sp;  SetObject(g->sp, receiver ); 
        runInterpreter(g, method, 1);
        g->canCallOS = false;
    pthread_mutex_unlock (&gLangMutex);
}



    
@end